:: Set Param
@ECHO OFF
@SETLOCAL

:: run Win x64
pushd build-win
OcrLiteOnnx.exe ../test/1.jpg ../models 1.0 0.6 0.3 3 1.3 1.3 1.6 1.6
popd
PAUSE
@ENDLOCAL
