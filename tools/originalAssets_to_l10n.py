"""
Just an ephemeral tool, it converts existing assets language file
to something compatible with json2po.
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
from pathlib import Path
from pathlib import PurePath

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

            us_messages = []
            gb_messages = []
            jp_messages = []
            it_messages = []
            fr_messages = []
            de_messages = []
            es_messages = []

            for item in langData:
                msg = {}
                
                msg["id"] = item["id"]
                msg["context"] = item["id"]
                
                #en-US
                for usItem in us_langData: #Probably a better way to do that but I was lazy, sorry
                    if usItem["id"] == msg["id"]:
                        msg["string"] = usItem["en"]
                        break
                us_messages.append(msg.copy())

                #en-GB
                msg["string"] = item["gb"]
                gb_messages.append(msg.copy())

                #ja-JP
                for jpItem in jp_langData: #yep, still lazy
                    if jpItem["id"] == msg["id"]:
                        msg["string"] = jpItem["jp"]
                        break
                jp_messages.append(msg.copy())

                #it-IT
                msg["string"] = item["it"]
                it_messages.append(msg.copy())

                #fr-FR
                msg["string"] = item["fr"]
                fr_messages.append(msg.copy())

                #de-De
                msg["string"] = item["de"]
                de_messages.append(msg.copy())

                #es-ES
                msg["string"] = item["es"]
                es_messages.append(msg.copy())
            
            os.makedirs(PurePath(L10N_BASE_LOCATION,"en_US", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"en_GB", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"ja_JP", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"it_IT", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"fr_FR", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"de_DE", textdomain), exist_ok=True)
            os.makedirs(PurePath(L10N_BASE_LOCATION,"es_ES", textdomain), exist_ok=True)            

            with open(PurePath(L10N_BASE_LOCATION,"en_US", textdomain, f"{fileName}"), "w", encoding="UTF-8") as new_file:
                data = {"messages":us_messages}
                json.dump(data, new_file, ensure_ascii=False, indent=4)

            with open(PurePath(L10N_BASE_LOCATION,"en_GB", textdomain, f"{fileName}"), "w", encoding="UTF-8") as new_file:
                data = {"messages":gb_messages}
                json.dump(data, new_file, ensure_ascii=False, indent=4)

            with open(PurePath(L10N_BASE_LOCATION,"ja_JP", textdomain, f"{fileName}"), "w", encoding="UTF-8") as new_file:
                data = {"messages":jp_messages}
                json.dump(data, new_file, ensure_ascii=False, indent=4)

            with open(PurePath(L10N_BASE_LOCATION,"it_IT", textdomain, f"{fileName}"), "w", encoding="UTF-8") as new_file:
                data = {"messages":it_messages}
                json.dump(data, new_file, ensure_ascii=False, indent=4)

            with open(PurePath(L10N_BASE_LOCATION,"fr_FR", textdomain, f"{fileName}"), "w", encoding="UTF-8") as new_file:
                data = {"messages":fr_messages}
                json.dump(data, new_file, ensure_ascii=False, indent=4)

            with open(PurePath(L10N_BASE_LOCATION,"de_DE", textdomain, f"{fileName}"), "w", encoding="UTF-8") as new_file:
                data = {"messages":de_messages}
                json.dump(data, new_file, ensure_ascii=False, indent=4)

            with open(PurePath(L10N_BASE_LOCATION,"es_ES", textdomain, f"{fileName}"), "w", encoding="UTF-8") as new_file:
                data = {"messages":es_messages}
                json.dump(data, new_file, ensure_ascii=False, indent=4)



if __name__ == "__main__":
    main()