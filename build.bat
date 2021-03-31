@echo off
setlocal EnableDelayedExpansion

if not exist obj mkdir obj
if not exist build mkdir build

set filenames=
for /R %%f in (*.c) do (
  if "%%~xf"==".c" set filenames=!filenames! "%%f"
)

set resfile=physarum

echo Files:%filenames%

set compiler_flags=/Fo.\obj\ -MT /EHsc -WL -Od -FC -Zi
set include_flags=/I .\src\ /I .\res\
set linker_flags=/link -incremental:no -opt:ref .\obj\%resfile%.res user32.lib gdi32.lib winmm.lib kernel32.lib opengl32.lib /SUBSYSTEM:WINDOWS /out:.\build\physarum.exe
set define_flags=-D_DEBUG -D_CRT_SECURE_NO_WARNINGS -D_CONSOLE -DPH_INTERNAL

rc /fo .\obj\%resfile%.res .\res\%resfile%.rc
cl %compiler_flags% %define_flags% %filenames% %include_flags% /DEBUG %linker_flags%

echo Error: %ERRORLEVEL%