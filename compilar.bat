@echo off
echo =============================================
echo  COMPILANDO AnalisadorSemantico (Fase 3)
echo =============================================

g++ -std=c++17 -Wall -Wextra -Isrc src/Fase1_Lexico.cpp src/LexerInterface.cpp src/ParserLL1.cpp src/ParserAST.cpp src/AnalisadorSemantico.cpp src/main.cpp -o AnalisadorSemantico.exe

if %ERRORLEVEL% == 0 (
    echo.
    echo [OK] Compilacao concluida! Executavel: AnalisadorSemantico.exe
    echo Uso: .\AnalisadorSemantico.exe teste1.txt
) else (
    echo.
    echo [FALHA] Erros de compilacao detectados.
)
pause
