# Build configurations used for external projects

## ffmpeg linking script
```
for %i in (*.def) do @for /f "delims=-" %j in ("%i") do @lib /nologo /def:%i /out:%j.lib
```
