"""
Just an ephemeral tool, it converts existing assets language file
to pofile.
I leave it here just in case it will be needed again for whatever reason.

So, we know that:
- We have 7 languages en-US, en-GB, ja-JP, it-IT, fr-FR, de-De, es-ES
- All version can theorically support all 7 languages
- But ntsc-final and jpn-final has only one language shipped
- pal-final has 5 languages supported (en-US and ja-JP are missing)

We can conclude that pal-final is theorically the most complete and that's why it will be our base.
So we will iterate through all json file in pal-final/lang. Based on the id, we will get ntsc-final and jpn-final equivalent
and make the final file.
"""
import glob
import json
import os
import polib
from pathlib import Path
from pathlib import PurePath
from datetime import datetime, timezone

PAL_SOURCE_FILE_LOCATION = "src/assets/pal-final/lang"
US_SOURCE_FILE_LOCATION = "src/assets/ntsc-final/lang"
JPN_SOURCE_FILE_LOCATION = "src/assets/jpn-final/lang"

L10N_BASE_LOCATION = "src/assets/l10n"



def main():
    for jsonFile in glob.glob(f"{PAL_SOURCE_FILE_LOCATION}/*.json"):

        with open(jsonFile, "r") as langFile:
            langData = json.load(langFile)
            us_langData = None
            jp_langData = None

            fileName = Path(jsonFile).name
            textdomain = Path(jsonFile).stem
            print(f"Reading {jsonFile}")

            with open(PurePath(US_SOURCE_FILE_LOCATION, fileName)) as usLang:
                us_langData = json.load(usLang)
            with open(PurePath(JPN_SOURCE_FILE_LOCATION, fileName)) as jpLang:
                jp_langData = json.load(jpLang)

            us_messages = polib.POFile()

            gb_messages = polib.POFile()

            jp_messages = polib.POFile()

            it_messages = polib.POFile()

            fr_messages = polib.POFile()

            de_messages = polib.POFile()

            es_messages = polib.POFile()

            for item in langData:

                msgid = ""
                skip_entry = False

                #en-US
                msg = polib.POEntry()
                id = item["id"]

                msg.msgctxt = item["id"]
                found = False # sometimes there is not entry in ntsc-final but they exists in pal
                for usItem in us_langData: # Probably a better way to do that but I was lazy, sorry
                    if usItem["id"] == id:
                        found = True
                        if usItem["en"] is None: # skipping empty string and entries
                            skip_entry = True
                            break
                        msgid = polib.escape(usItem["en"])
                        msg.msgid = polib.escape(usItem["en"])
                        msg.msgstr = polib.escape(usItem["en"])
                        break
                
                if not skip_entry and found:
                    us_messages.append(msg)

                    #en-GB
                    msg = polib.POEntry()
                    msg.msgctxt = item["id"]
                    msg.msgid = msgid
                    msg.msgstr = polib.escape(item["gb"])
                    gb_messages.append(msg)

                    #ja-JP
                    msg = polib.POEntry()
                    msg.msgctxt = item["id"]
                    msg.msgid = msgid
                    for jpItem in jp_langData: #yep, still lazy
                        if jpItem["id"] == id:
                            msg.msgstr = polib.escape(jpItem["jp"])
                            break
                    jp_messages.append(msg)

                    #it-IT
                    msg = polib.POEntry()
                    msg.msgctxt = item["id"]
                    msg.msgid = msgid
                    msg.msgstr = polib.escape(item["it"])
                    it_messages.append(msg)

                    #fr-FR
                    msg = polib.POEntry()
                    msg.msgctxt = item["id"]
                    msg.msgid = msgid
                    msg.msgstr = polib.escape(item["fr"])
                    fr_messages.append(msg)

                    #de-De
                    msg = polib.POEntry()
                    msg.msgctxt = item["id"]
                    msg.msgid = msgid
                    msg.msgstr = polib.escape(item["de"])
                    de_messages.append(msg)

                    #es-ES
                    msg = polib.POEntry()
                    msg.msgctxt = item["id"]
                    msg.msgid = msgid
                    msg.msgstr = polib.escape(item["es"])
                    es_messages.append(msg)

            os.makedirs(PurePath(L10N_BASE_LOCATION,"en_US", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"en_GB", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"ja_JP", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"it_IT", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"fr_FR", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"de_DE", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"es_ES", textdomain), exist_ok=True)

            us_messages.save(PurePath(L10N_BASE_LOCATION,"en_US", textdomain, f"{textdomain}.po"))
            gb_messages.save(PurePath(L10N_BASE_LOCATION,"en_GB", textdomain, f"{textdomain}.po"))
            jp_messages.save(PurePath(L10N_BASE_LOCATION,"ja_JP", textdomain, f"{textdomain}.po"))
            it_messages.save(PurePath(L10N_BASE_LOCATION,"it_IT", textdomain, f"{textdomain}.po"))
            fr_messages.save(PurePath(L10N_BASE_LOCATION,"fr_FR", textdomain, f"{textdomain}.po"))
            de_messages.save(PurePath(L10N_BASE_LOCATION,"de_DE", textdomain, f"{textdomain}.po"))
            es_messages.save(PurePath(L10N_BASE_LOCATION,"es_ES", textdomain, f"{textdomain}.po"))

            # I don't know why but polib append empty string when saving the file
            # The entry doesn't even exists in xx_messages object
            # so we will remove the 3 first lines
            # I will also remove empty file here
            
            # en-US
            lines = []
            path = PurePath(L10N_BASE_LOCATION,"en_US", textdomain, f"{textdomain}.po")
            with open(path, "r") as f:
                lines = f.readlines()
            with open(path, "w") as f:
                for n, line in enumerate(lines):
                    if n > 2:
                        f.write(line)
            if os.stat(path).st_size == 0:
                os.remove(path)

            # en-GB
            lines = []
            path = PurePath(L10N_BASE_LOCATION,"en_GB", textdomain, f"{textdomain}.po")
            with open(path, "r") as f:
                lines = f.readlines()
            with open(path, "w") as f:
                for n, line in enumerate(lines):
                    if n > 2:
                        f.write(line)
            if os.stat(path).st_size == 0:
                os.remove(path)
            
            # ja-JP
            lines = []
            path = PurePath(L10N_BASE_LOCATION,"ja_JP", textdomain, f"{textdomain}.po")
            with open(path, "r") as f:
                lines = f.readlines()
            with open(path, "w") as f:
                for n, line in enumerate(lines):
                    if n > 2:
                        f.write(line)
            if os.stat(path).st_size == 0:
                os.remove(path)
            
            # it-IT
            lines = []
            path = PurePath(L10N_BASE_LOCATION,"it_IT", textdomain, f"{textdomain}.po")
            with open(path, "r") as f:
                lines = f.readlines()
            with open(path, "w") as f:
                for n, line in enumerate(lines):
                    if n > 2:
                        f.write(line)
            if os.stat(path).st_size == 0:
                os.remove(path)
            
            # fr-FR
            lines = []
            path = PurePath(L10N_BASE_LOCATION,"fr_FR", textdomain, f"{textdomain}.po")
            with open(path, "r") as f:
                lines = f.readlines()
            with open(path, "w") as f:
                for n, line in enumerate(lines):
                    if n > 2:
                        f.write(line)
            if os.stat(path).st_size == 0:
                os.remove(path)
            
            # de-DE
            lines = []
            path = PurePath(L10N_BASE_LOCATION,"de_DE", textdomain, f"{textdomain}.po")
            with open(path, "r") as f:
                lines = f.readlines()
            with open(path, "w") as f:
                for n, line in enumerate(lines):
                    if n > 2:
                        f.write(line)
            if os.stat(path).st_size == 0:
                os.remove(path)
            
            # es-ES
            lines = []
            path = PurePath(L10N_BASE_LOCATION,"es_ES", textdomain, f"{textdomain}.po")
            with open(path, "r") as f:
                lines = f.readlines()
            with open(path, "w") as f:
                for n, line in enumerate(lines):
                    if n > 2:
                        f.write(line)
            if os.stat(path).st_size == 0:
                os.remove(path)



if __name__ == "__main__":
    main()