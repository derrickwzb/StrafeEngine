@echo off

set "dependencies_path=%~dp0\..\Strafe\dependencies"
echo [97mPulling all dependencies[0m

call :pull_dependency "spdlog" "https://github.com/derrickwzb/spdlog.git"
call :pull_dependency "glfw" "https://github.com/derrickwzb/glfw.git"
call :pull_dependency "glad" "https://github.com/derrickwzb/Glad.git"
call :pull_dependency "glm" "https://github.com/derrickwzb/glm.git"
call :pull_dependency "entt" "https://github.com/derrickwzb/entt.git"

set "dependencies_path=%~dp0\..\Editor\dependencies"
call :pull_dependency "imgui" "--branch docking https://github.com/derrickwzb/imgui.git"

call :exit
pause >nul
exit

:pull_dependency
echo [97mCloning %~1[0m
if exist "%dependencies_path%\%~1" (
    echo [93m%~1 exists. Delete the folder if updating is required.[0m
) else (
    git clone %~2 "%dependencies_path%\%~1" 
    if errorlevel 1 (
       echo [91mError pulling %~1 from %~2[0m
    ) else (
       echo [92m%~1 cloned succesfully.[0m
    )
)
goto:eof

:exit
echo [97mScript done[0m
goto:eof