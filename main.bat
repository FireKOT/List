@call parameters.bat main.cpp list.cpp -o main.exe

@if not errorlevel 1 (
    main.exe %*
)