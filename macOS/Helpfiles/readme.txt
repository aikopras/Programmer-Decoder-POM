The folder "Decoder-POMHelp" contains the (editable) sources of the help files.
The help pages are plain (X)HTML files, and can be edited with any text editor or within Xcode.


How the help files are included in the application:
- The application uses the copy in "Programmer Decoder-POM/Decoder-POMHelp".
- In Xcode that folder is included under "Supporting Files" as a folder reference (blue folder icon),
  and is part of "Copy Bundle Resources" (select the target, tab "Build Phases").
  As a result, the folder is copied as a whole to "Contents/Resources/Decoder-POMHelp" of the application.
- "Programmer Decoder-POM-Info.plist" refers to this folder:
  - Help Book directory name (CFBundleHelpBookFolder): Decoder-POMHelp
  - Help Book identifier (CFBundleHelpBookName): Decoder-POM Help
  The Help Book identifier must be equal to the AppleTitle in index.html.
- The files in "Helpfiles/Decoder-POMHelp" are only shown in Xcode (group "Helpfiles");
  they are not part of the application (no target membership).


To modify existing help files, or to add new ones:
1) Modify the files in "Helpfiles/Decoder-POMHelp"
   - For a new page: add a link to that page in index.html
2) Copy all files (in the Finder) to "Programmer Decoder-POM/Decoder-POMHelp"
3) In Xcode: Product -> Clean Build Folder, followed by Build
There is no need to remove and add the folder in Xcode, since all files within a folder reference
are copied, including new files.


Note: do NOT add the help folder as a group (yellow folder icon), and do NOT add the help pages as
individual files to "Copy Bundle Resources". In that case the files are copied separately to
"Contents/Resources", the Help menu can not find them, and files with the same name result in
"Multiple commands produce ..." build errors.
