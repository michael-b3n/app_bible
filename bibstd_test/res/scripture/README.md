# Scripture test data

Drop scripture archives here for the `scripture_usx` and `core_scripture_store` tests. These are the USX zip bundles downloaded from the Digital Bible Library at [library.bible](https://library.bible/), the same files the app loads from its scripture folder.

Neither the archives nor anything extracted from them are committed, `.gitignore` keeps this folder empty except for its documentation. Without archives the tests skip.

`scripture_usx` checks book names of specific bundles, it expects:

| File | Translation |
|---|---|
| `text-542b32484b6e38c2-246437.zip` | Textbibel von Kautzsch und Weizsäcker |
| `text-de4e12af7f28f599-245514.zip` | King James Version |
| `text-f492a38d0e52db0f-258505.zip` | Elberfelder Übersetzung (Version von bibelkommentare.de) |
