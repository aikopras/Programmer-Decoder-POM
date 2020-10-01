The folder "Decoder-POMHelp" contains the (editable) sources of the help files.


To add these help files to the application code:
1) Select with the mouse the folder "Supporting files" in the Project navigator at the left side of the Xcode window
2) From the Xcode "File menu", select "Add Files to ..."
   - Select the folder "Decoder-POMHelp" from the Helpfiles folder 
   - Make sure to select "Create folder references for any added folders"
   
   
To modify existing help files:
1) Modify the source files in the subfolder "Decoder-POMHelp" of "Helpfiles"
2) Delete to Trash the subfolder "Decoder-POMHelp" under "Supporting files" 
3) Perform the steps described above under "To add ..."


If help files are added first to the project:
1) Edit "Programmer Decoder-POM-Info.plist" and add two rows:
   - Help book directory name
   - Help book identifier
2) Select with the mouse the "target" (Programmer Decoder-POM) in the Project navigator at the left side of the Xcode
   - Select the "Build Phases" tab
   - Edit (if needed) the "Copy Bundle Resources"

Note: TextEdit can be used to move from keynote tables to HTML files
