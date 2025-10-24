@echo off
REM 引数でコミットメッセージを受け取る
REM 例: gitPus.bat "変更内容"

if "%~1"=="" (
    echo コミットメッセージを指定してください。
    echo 例: gitPush.bat "更新内容"
    exit /b 1
)

git add .
git commit -m "commit_%~1"
git push -u origin main

