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

L10N_BASE_LOCATION = "src/assets/i18n/locale"
#L_TITLE_154
poMeta = {
    'Project-Id-Version': 'Perfect Dark PC port localization',
    'POT-Creation-Date': f'{datetime.now(timezone.utc)}',
    'MIME-Version': '1.0',
    'Content-Type': 'text/plain; charset=utf-8',
    'Content-Transfer-Encoding': '8bit',
    'X-Generator' : 'originalAssets_to_l10n.py script'
}


def main():
    for jsonFile in glob.glob(f"{PAL_SOURCE_FILE_LOCATION}/*.json"):

        with open(jsonFile, "r") as langFile:
            langData = json.load(langFile)
            if len(langData) == 0:
                continue
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
            us_messages.metadata = poMeta

            gb_messages = polib.POFile()
            gb_messages.metadata = poMeta

            jp_messages = polib.POFile()
            jp_messages.metadata = poMeta

            it_messages = polib.POFile()
            it_messages.metadata = poMeta

            fr_messages = polib.POFile()
            fr_messages.metadata = poMeta

            de_messages = polib.POFile()
            de_messages.metadata = poMeta

            es_messages = polib.POFile()
            es_messages.metadata = poMeta

            for item in langData:

                msgid = ""
                skip_entry = False

                #en-US
                msg = polib.POEntry()
                id = item["id"]

                msg.comment = item["id"]
                found = False # sometimes there is not entry in ntsc-final but they exists in pal
                for usItem in us_langData: # Probably a better way to do that but I was lazy, sorry
                    if usItem["id"] == id:
                        found = True
                        if usItem["en"] is None: # skipping empty string and entries
                            skip_entry = True
                            break
                        msgid = usItem["en"]
                        msg.msgid = usItem["en"]
                        msg.msgstr = usItem["en"]
                        break
                
                if not found:
                    msgid = item["en"]
                    if msgid is None:
                        skip_entry = True
                        found = False
                    else:
                        msg.msgid = item["en"]
                        msg.msgstr = item["en"]
                        found = True
                if not skip_entry and found:
                    us_messages.append(msg)

                    #en-GB
                    msg = polib.POEntry()
                    msg.comment = item["id"]
                    msg.msgid = msgid
                    msg.msgstr = item["gb"]
                    gb_messages.append(msg)

                    #ja-JP
                    msg = polib.POEntry()
                    msg.comment = item["id"]
                    msg.msgid = msgid
                    for jpItem in jp_langData: #yep, still lazy
                        if jpItem["id"] == id:
                            msg.msgstr = jpItem["jp"]
                            break
                    jp_messages.append(msg)

                    #it-IT
                    msg = polib.POEntry()
                    msg.comment = item["id"]
                    msg.msgid = msgid
                    msg.msgstr = item["it"]
                    it_messages.append(msg)

                    #fr-FR
                    msg = polib.POEntry()
                    msg.comment = item["id"]
                    msg.msgid = msgid
                    msg.msgstr = item["fr"]
                    fr_messages.append(msg)

                    #de-De
                    msg = polib.POEntry()
                    msg.comment = item["id"]
                    msg.msgid = msgid
                    msg.msgstr = item["de"]
                    de_messages.append(msg)

                    #es-ES
                    msg = polib.POEntry()
                    msg.comment = item["id"]
                    msg.msgid = msgid
                    msg.msgstr = item["es"]
                    es_messages.append(msg)

            os.makedirs(PurePath(L10N_BASE_LOCATION,"en_US"), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"en_GB"), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"ja_JP"), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"it_IT"), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"fr_FR"), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"de_DE"), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"es_ES"), exist_ok=True)

            for entry in us_messages:
                if(entry.msgid is None):                    
                    entry.msgid = "<None>"
                    us_messages.remove(entry)
            for entry in gb_messages:
                if(entry.msgid is None):
                   entry.msgid = "<None>"
                   gb_messages.remove(entry)
            for entry in jp_messages:
                if(entry.msgid is None):
                    entry.msgid = "<None>"
                    jp_messages.remove(entry)
            for entry in it_messages:
                if(entry.msgid is None):
                    entry.msgid = "<None>"
                    it_messages.remove(entry)
            for entry in fr_messages:
                if(entry.msgid is None):
                    entry.msgid = "<None>"
                    fr_messages.remove(entry)
            for entry in de_messages:
                if(entry.msgid is None):
                    entry.msgid = "<None>"
                    de_messages.remove(entry)
            for entry in es_messages:
                if(entry.msgid is None):
                    entry.msgid = "<None>"
                    es_messages.remove(entry)

            #us_messages.save(PurePath(L10N_BASE_LOCATION,"en_US", f"{textdomain}.po"))
            #gb_messages.save(PurePath(L10N_BASE_LOCATION,"en_GB", f"{textdomain}.po"))
            #jp_messages.save(PurePath(L10N_BASE_LOCATION,"ja_JP", f"{textdomain}.po"))
            #it_messages.save(PurePath(L10N_BASE_LOCATION,"it_IT", f"{textdomain}.po"))
            #fr_messages.save(PurePath(L10N_BASE_LOCATION,"fr_FR", f"{textdomain}.po"))
            #de_messages.save(PurePath(L10N_BASE_LOCATION,"de_DE", f"{textdomain}.po"))
            #es_messages.save(PurePath(L10N_BASE_LOCATION,"es_ES", f"{textdomain}.po"))


if __name__ == "__main__":
    main()
