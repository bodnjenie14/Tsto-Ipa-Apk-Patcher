plist_path = "decipa/Payload/Tapped Out.app/Info.plist"  # Pfad zur Info.plist-Datei
        new_mayhem_url = patched_url  # Neuer Wert für MayhemServerURL
        new_dlc_location = patched_url2.decode('utf-8', errors='ignore')  # Neuer Wert für DLCLocation
        try:
            # Lese die PLIST-Datei ein
            with open(plist_path, 'rb') as plist_file:
                plist_data = plistlib.load(plist_file)

            # Überprüfe, ob die Schlüssel existieren, und aktualisiere die <string>-Werte
            if 'MayhemServerURL' in plist_data:
                plist_data['MayhemServerURL'] = new_mayhem_url
            else:
                print("Der Schlüssel 'MayhemServerURL' wurde nicht gefunden.")

            if 'DLCLocation' in plist_data:
                plist_data['DLCLocation'] = new_dlc_location
            else:
                print("Der Schlüssel 'DLCLocation' wurde nicht gefunden.")

            # Speichere die aktualisierte PLIST-Datei
            with open(plist_path, 'wb') as plist_file:
                plistlib.dump(plist_data, plist_file)

            print(f"Die Datei {plist_path} wurde erfolgreich aktualisiert.")
            print(f"Neuer MayhemServerURL: {new_mayhem_url}")
            print(f"Neuer DLCLocation: {new_dlc_location}")

            file_path = "decipa/Payload/Tapped Out.app/Tapped Out"  # Pfad zur Datei
            old_urls = [
                "http://oct2018-4-35-0-uam5h44a.tstodlc.eamobile.com/netstorage/gameasset/direct/simpsons/",
                "https://syn-dir.sn.eamobile.com"
            ]
            new_urls = [
                patched_url2.decode('utf-8', errors='ignore'),  # Neue URL für die erste alte URL
                patched_url  # Neue URL für die zweite alte URL
            ]

            with open(file_path, 'rb') as file:
                content = bytearray(file.read())  # In ein bytearray umwandeln, um Änderungen vorzunehmen

            # Ersetze die URLs
            for old_url, new_url in zip(old_urls, new_urls):
                # Stelle sicher, dass die neue URL die gleiche Länge wie die alte URL hat
                if len(new_url) < len(old_url):
                    # Fülle die neue URL mit '/' auf
                    new_url = new_url.ljust(len(old_url), '/')
                elif len(new_url) > len(old_url):
                    raise ValueError(f"Die neue URL '{new_url}' ist länger als die alte URL '{old_url}'.")

                # Konvertiere die URLs in Bytes
                old_url_bytes = old_url.encode('utf-8')
                new_url_bytes = new_url.encode('utf-8')

                # Suche und ersetze die URLs im Binärinhalt
                content = content.replace(old_url_bytes, new_url_bytes)

            # Speichere die aktualisierte Binärdatei
            with open(file_path, 'wb') as file:
                file.write(content)

            print(f"Die Datei {file_path} wurde erfolgreich aktualisiert.")
            for old_url, new_url in zip(old_urls, new_urls):
                print(f"Ersetzt: {old_url} -> {new_url}")

        except FileNotFoundError:
            print(f"Die Datei {plist_path} wurde nicht gefunden.")
        except Exception as e:
            print(f"Ein Fehler ist aufgetreten: {e}")