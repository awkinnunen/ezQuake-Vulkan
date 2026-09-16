@echo off
cd /d "%~dp0"
FriendsProbe.exe --host --copy-invite --invite-out invitation.txt --report host-result.json --seconds 0
echo.
echo Test finished. Share host-result.json for diagnosis, not invitation.txt.
pause
