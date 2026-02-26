#include <iostream>
#include <string>
#include <list>
#include "../include/tokenizador.h" // Asegúrate de que la ruta sea correcta

using namespace std;

int main(void) {
    // 1. Configuración del tokenizador
    // Delimitadores por defecto (puedes cambiarlos según necesites)
    string delimitadores = ",;:.-/+*\\ '\"{}[]()<>¡!¿?&#=\t@";
    bool kCasosEspeciales = true;
    bool kPasarAminusculas = true;

    // 2. Creación de la instancia
    Tokenizador a(delimitadores, kCasosEspeciales, kPasarAminusculas);

    cout << "Configuración actual: " << a << endl;
    cout << "Iniciando la tokenización del directorio 'corpus'..." << endl;

    // 3. Llamada al método que procesa el directorio
    // Este método buscará todos los archivos dentro de "corpus",
    // los ordenará y creará un archivo .tk por cada uno.
    if (a.TokenizarDirectorio("corpus")) {
        cout << "Proceso finalizado con éxito." << endl;
        cout << "Se han generado los archivos .tk correspondientes en el directorio 'corpus'." << endl;
    } else {
        cerr << "Hubo un error al procesar el directorio o el directorio no existe." << endl;
        return 1;
    }

    return 0;
}