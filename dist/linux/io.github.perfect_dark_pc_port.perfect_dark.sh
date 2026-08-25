#!/bin/sh

# Create user data directories if necessary
userdirs=( roms saves mods )
for path in "${userdirs[@]}"; do
    if [ ! -d "${XDG_DATA_HOME}/${path}" ]; then
        mkdir -p "${XDG_DATA_HOME}/${path}"
    fi
done

# Default to pd as executable but switch to jpn/pal if the NTSC rom
# is missing but JPN/PAL are present.
# TODO: Other arch
executable="pd.x86_64"
if [ -f ${XDG_DATA_HOME}/roms/pd.ntsc-final.z64 ] | \
   [ -f ${XDG_DATA_HOME}/roms/pd.ntsc-1.0.z64 ]; then
    executable="pd.x86_64"
elif [ -f ${XDG_DATA_HOME}/roms/pd.jpn-final.z64 ]; then
    executable="pd.jpn.x86_64"
elif [ -f ${XDG_DATA_HOME}/roms/pd.pal-final.z64 ]; then
    executable="pd.pal.x86_64"
fi

# If first parameter passed to this script is one of the pd executables,
# run that regardless of ROM auto-detection above and pass the remaining
# parameters to it; otherwise pass all arguments to the executable selected
# above
case $1 in
    "pd."*)
        executable=$1
        params="${@:2}"
        ;;
    *)
        params="$@"
        ;;
esac

# Run game
$executable --basedir ${XDG_DATA_HOME}/roms --savedir ${XDG_DATA_HOME}/saves $params
