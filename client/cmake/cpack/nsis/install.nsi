DetailPrint "Registering Sharity URL"
DeleteRegKey HKCR "sharity"
WriteRegStr HKCR "sharity" "" "URL:sharity"
WriteRegStr HKCR "sharity" "URL Protocol" ""
WriteRegStr HKCR "sharity/DefaultIcon" "$INSTDIR\\sharity-client.exe"
WriteRegStr HKCR "sharity/shell" "" ""
WriteRegStr HKCR "sharity/shell/open" "" ""
WriteRegStr HKCR "sharity/shell/open/command" "" "$INSTDIR\sharity-client.exe %1"

