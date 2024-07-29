@echo off
for %%i in (*.csv) do (
    type nul > "%%~ni_exam.csv"
)