@echo off
set MAKE="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\nmake.exe"
set PATH=%PATH%;%EMSCRIPTEN%\bin
call "C:\Program Files\Emscripten\emsdk_env.bat"
call :build_assimp
call :build_bullet
call :build
goto :eof

:build_assimp
mkdir extern\assimp-build-js
mkdir extern\assimp-install-js
cd extern\assimp-build-js
call emcmake cmake^
 -DCMAKE_MAKE_PROGRAM=%MAKE%^
 -DCMAKE_INSTALL_PREFIX=..\assimp-install-js^
 -G "Unix Makefiles"^
 -DASSIMP_BUILD_TESTS=OFF^
 ..\assimp
call %MAKE% -j %NUMBER_OF_PROCESSORS%
call %MAKE% install
cd ..\..
goto :eof

:build_bullet
mkdir extern\bullet3-build-js
mkdir extern\bullet3-install-js
cd extern\bullet3-build-js
call emcmake cmake^
 -DCMAKE_MAKE_PROGRAM=%MAKE%^
 -DCMAKE_INSTALL_PREFIX=..\bullet3-install-js^
 -G "Unix Makefiles"^
 -DBUILD_SHARED_LIBS=ON^
 -DBUILD_DEMOS=OFF^
 -DBUILD_UNIT_TESTS=OFF^
 -DBUILD_CPU_DEMOS=OFF^
 -DBUILD_BULLET2_DEMOS=OFF^
 -DBUILD_EXTRAS=OFF^
 -DCMAKE_INSTALL_PREFIX=../bullet3-install^
 ../bullet3
call %MAKE% -j %NUMBER_OF_PROCESSORS%
call %MAKE% install
cd ..\..
goto :eof

:build
mkdir build-html install-html
cd build-html
call emcmake cmake^
 -DCMAKE_MAKE_PROGRAM=%MAKE%^
 -DCMAKE_INSTALL_PREFIX=..\install-html^
 -G "Unix Makefiles"^
 -DEMSCRIPTEN_ENABLED=ON^
 ..
call %MAKE% -j %NUMBER_OF_PROCESSORS%
call %MAKE% install
cd ..
echo "Done!"
goto :eof
