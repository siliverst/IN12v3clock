@echo off
setlocal enabledelayedexpansion

:: ============================================================
:: НАСТРОЙКИ (Измените под ваш микроконтроллер)
:: ============================================================
:: Размер RAM в байтах (например: 1024 для STM8S003/103, 2048 для STM8S105)
set "TOTAL_RAM_SIZE=1024"

:: Размер ROM (Flash) в байтах (например: 8192 для STM8S003/103, 32768 для STM8S105)
set "TOTAL_ROM_SIZE=8192"
:: ============================================================

:: Получаем имя map-файла из аргументов
set "MAP_FILE=%~1"

if "%MAP_FILE%"=="" (
    echo [ERROR] Map file not specified!
    echo Usage: %~nx0 ^<filename.map^>
    exit /b 1
)

if not exist "%MAP_FILE%" (
    echo [ERROR] File not found: %MAP_FILE%
    exit /b 1
)

:: Переменные для подсчета
set "USED_RAM=0"
set "USED_ROM=0"

:: ------------------------------------------------------------
:: 1. АНАЛИЗ RAM
:: Ищем секции: .bsct (initialized), .ubsct, .data, .bss, .fbss
:: ------------------------------------------------------------
for /f "tokens=6" %%A in ('findstr /C:"segment .bsct, initialized" /C:"segment .ubsct" /C:"segment .data" /C:"segment .bss" /C:"segment .fbss" "%MAP_FILE%"') do (
    set "LEN=%%A"
    set /a USED_RAM+=!LEN!
)

:: ------------------------------------------------------------
:: 2. АНАЛИЗ ROM (Flash)
:: Ищем секции: 
:: .text (код)
:: .const (константы, может быть несколько секций)
:: .init (код инициализации cstartup)
:: .bsct, from (копия начальных значений переменных во Flash)
:: ------------------------------------------------------------
for /f "tokens=6" %%A in ('findstr /C:"segment .text" /C:"segment .const" /C:"segment .init" /C:"segment .bsct, from" "%MAP_FILE%"') do (
    set "LEN=%%A"
    set /a USED_ROM+=!LEN!
)

:: ------------------------------------------------------------
:: 3. ВЫЧИСЛЕНИЯ
:: ------------------------------------------------------------

:: RAM
set /a FREE_RAM=%TOTAL_RAM_SIZE% - %USED_RAM%
set /a PERC_RAM=(%USED_RAM% * 100) / %TOTAL_RAM_SIZE%

:: ROM
set /a FREE_ROM=%TOTAL_ROM_SIZE% - %USED_ROM%
set /a PERC_ROM=(%USED_ROM% * 100) / %TOTAL_ROM_SIZE%

:: ------------------------------------------------------------
:: 4. ВЫВОД РЕЗУЛЬТАТА
:: ------------------------------------------------------------
echo.
echo ======================================================
echo           MEMORY USAGE REPORT: %~nx1
echo ======================================================
echo   Total RAM: %TOTAL_RAM_SIZE% bytes
echo   Used RAM:  %USED_RAM% bytes (%PERC_RAM%%%)
echo   Free RAM:  %FREE_RAM% bytes (Stack + Heap)
echo ------------------------------------------------------
echo   Total ROM: %TOTAL_ROM_SIZE% bytes
echo   Used ROM:  %USED_ROM% bytes (%PERC_ROM%%%)
echo   Free ROM:  %FREE_ROM% bytes
echo ======================================================
exit /b 0
