import os
import sys
import tempfile
import subprocess

def align4(value):
    return (value + 3 | 3) ^ 3

def align16(value):
    return (value + 15 | 15) ^ 15

def pad4(binary):
    if len(binary) % 4:
        amount = 4 - (len(binary) % 4)
        binary += (0).to_bytes(amount, 'big')

    return binary

def pad16(binary):
    if len(binary) % 16:
        amount = 16 - (len(binary) % 16)
        binary += (0).to_bytes(amount, 'big')

    return binary

def write_enums(typename, names, filename, terminator, start=0):
    filename = 'src/generated/%s/%s' % (os.environ['ROMID'], filename)
    mkpath(filename)

    fd = open(filename, 'w')
    fd.write('/**\n')
    fd.write(' * This file was generated automatically. Changes may be overwritten.\n')
    fd.write(' */\n')
    fd.write('\n')
    fd.write('enum %s {\n' % typename)

    for index, name in enumerate(names):
        if index == 0 and start != 0:
            fd.write('\t%s = 0x%x,\n' % (name, start))
        else:
            fd.write('\t%s,\n' % name)

    fd.write('\t%s\n' % terminator)
    fd.write('};\n')

    fd.close()

def write_data(data):
    idx = sys.argv.index('--raw-out')
    filename = sys.argv[idx + 1]
    print('writing binary to', filename)
    fd = open(filename, 'wb')
    fd.write(data)
    fd.close()

def write_object(data, filename):
    if '--raw-out' in sys.argv:
        # write the raw data into a user-specified file instead
        write_data(data)
        return

    filename = 'build/%s/assets/%s' % (os.environ['ROMID'], filename)
    mkpath(filename)
    fd = open(filename, 'wb')

    # 0x00 - ELF header
    fd.write(b'\x7f\x45\x4c\x46\x01\x02\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    fd.write(b'\x00\x01\x00\x08\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00')
    fd.write(b'\x00\x00\x00\x40\x10\x00\x10\x00\x00\x34\x00\x00\x00\x00\x00\x28')
    fd.write(b'\x00\x03\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')

    # 0x40 - Section table index 0 (blank)
    fd.write(b'\x00\x00\x00\x00') # offset to section name
    fd.write(b'\x00\x00\x00\x00') # type
    fd.write(b'\x00\x00\x00\x00') # attributes (writeable and occupies memory during execution)
    fd.write(b'\x00\x00\x00\x00') # virtual address
    fd.write(b'\x00\x00\x00\x00') # offset to data
    fd.write(b'\x00\x00\x00\x00') # length
    fd.write(b'\x00\x00\x00\x00') # section index
    fd.write(b'\x00\x00\x00\x00') # extra information
    fd.write(b'\x00\x00\x00\x00') # alignment
    fd.write(b'\x00\x00\x00\x00') # entry size

    # 0x68 - Section table index 1 (.data)
    fd.write(b'\x00\x00\x00\x01') # offset to section name
    fd.write(b'\x00\x00\x00\x01') # type
    fd.write(b'\x00\x00\x00\x03') # attributes (writeable and occupies memory during execution)
    fd.write(b'\x00\x00\x00\x00') # virtual address
    fd.write(b'\x00\x00\x00\xd0') # offset to data
    fd.write(align16(len(data)).to_bytes(4, 'big'))
    fd.write(b'\x00\x00\x00\x00') # section index
    fd.write(b'\x00\x00\x00\x00') # extra information
    fd.write(b'\x00\x00\x00\x10') # alignment
    fd.write(b'\x00\x00\x00\x00') # entry size

    # 0x90 - Section table index 2 (.shstrtab)
    fd.write(b'\x00\x00\x00\x07') # offset to section name
    fd.write(b'\x00\x00\x00\x03') # type
    fd.write(b'\x00\x00\x00\x00') # attributes (writeable and occupies memory during execution)
    fd.write(b'\x00\x00\x00\x00') # virtual address
    fd.write(b'\x00\x00\x00\xb8') # offset to data
    fd.write(b'\x00\x00\x00\x18') # length
    fd.write(b'\x00\x00\x00\x00') # section index
    fd.write(b'\x00\x00\x00\x00') # extra information
    fd.write(b'\x00\x00\x00\x01') # alignment
    fd.write(b'\x00\x00\x00\x00') # entry size

    # 0xb8 = .shstrtab
    fd.write(b'\x00')
    fd.write('.data'.encode('utf8'))
    fd.write(b'\x00')
    fd.write('.shstrtab'.encode('utf8'))
    fd.write((0).to_bytes(8, 'big'))

    # 0xd0 = .data
    fd.write(data)

    # Pad to next 16-byte boundary
    amount = 16 - (len(data) % 16)
    fd.write((0).to_bytes(amount, 'big'))

    fd.close()

def zip(data):
    if 'win32' in sys.platform or 'cygwin' in sys.platform:
        filename = os.path.join(tempfile.gettempdir(), 'pd-assetmgr-%d' % os.getpid())
    else:
        filename = '/tmp/pd-assetmgr-%d' % os.getpid()

    fd = open(filename, 'wb')
    fd.write(data)
    fd.close()

    if sys.platform.startswith('linux'):
        # on Linux use supplied gzip 1.2.4
        gzip_args = ['tools/gzip', '-c', '--no-name', '--best', filename]
    else:
        # on other platforms use system gzip
        # NOTE: this might result in a non-matching file, but we don't really care
        gzip_args = ['gzip', '-f', '-c', '--no-name', '--best', filename]

    stream = subprocess.check_output(gzip_args)[10:-8]
    os.remove(filename)

    return b'\x11\x73' + len(data).to_bytes(3, 'big') + stream

def mkpath(filename):
    dirname = os.path.dirname(filename)

    if not os.path.isdir(dirname):
        os.makedirs(dirname, exist_ok=True)

def writefile(filename, contents):
    mkpath(filename)
    fd = open(filename, 'wb')
    fd.write(contents)
    fd.close()

if 'ROMID' not in os.environ:
    # check if it's in args
    for arg in sys.argv:
        if arg.startswith('--romid='):
            os.environ['ROMID'] = arg[len('--romid='):]
            break
    else:
        # default to ntsc-final
        os.environ['ROMID'] = 'ntsc-final'
