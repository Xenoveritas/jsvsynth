# Originally I was going to try and use the full path to 7-Zip via
# %ProgramFiles% but NMAKE is godawful and made that freaking impossible
# ('C:\Program' is not recognized as an internal or external command,
# operable program or batch file.)
# So now you have to add it to your PATH instead.
# Deal with it.

Build\jsvsynth.zip: Release/jsvsynth.dll
	CD Release && 7z a ..\Build\jsvsynth.zip jsvsynth.dll
	CD build && 7z u ..\Build\jsvsynth.zip docs\*.html docs\bootstrap docs\api
	7z u Build\jsvsynth.zip LICENSE.txt

Release/jsvsynth.dll: v8/build/Release/lib/v8_base.ia32.lib v8/build/Release/lib/v8_snapshot.lib
	MSBuild jsvsynth.sln /p:Configuration=Release

v8/build/Release/lib/v8_base.ia32.lib v8/build/Release/lib/v8_snapshot.lib:
	MSBuild v8\build\all.sln /p:Configuration=Release
