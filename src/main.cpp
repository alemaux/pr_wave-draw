#include "viewer.hpp"
#include <qapplication.h>
#include <windows.h>
#include <stdio.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

    int argc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);

    // Convertir LPWSTR en char** pour que QApplication puisse les accepter
    char** argv = new char*[argc];
    for (int i = 0; i < argc; ++i) {
        size_t size = wcslen(argvW[i]) + 1;
        argv[i] = new char[size];
        wcstombs(argv[i], argvW[i], size);
    }

    QApplication application(argc, argv);

    // Instancier le viewer
    Viewer viewer;

    viewer.setWindowTitle("Wave Viewer");
    viewer.treatArguments(argc, argv);

    viewer.show();

    // Exécuter la boucle principale
    return application.exec();
}
