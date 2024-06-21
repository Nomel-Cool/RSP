@echo off
for %%f in (*.csv) do (
    type nul > %%f
)