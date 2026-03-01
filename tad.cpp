#include <iostream>
#include <string>
#include <list>
#include <sys/resource.h>
#include "../include/tokenizador.h" // Asegúrate de que la ruta sea correcta

using namespace std;

double getcputime(void) {
    struct timeval tim;
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    tim=ru.ru_utime;
    double t=(double)tim.tv_sec + (double)tim.tv_usec / 1000000.0;
    tim=ru.ru_stime;
    t+=(double)tim.tv_sec + (double)tim.tv_usec / 1000000.0;
    return t;
}

void imprimirListaSTL(const list<string>& cadena)
{
        list<string>::const_iterator itCadena;
        for(itCadena=cadena.begin();itCadena!=cadena.end();itCadena++)
        {
                cout << (*itCadena) << ", ";
        }
        cout << endl;
}

int main(void) {

    long double aa=getcputime();
    Tokenizador a("\t ,;:.-+/*_`'{}[]()!?&#\"\\<>", true, true);
    a.TokenizarListaFicheros("listaFicheros.txt"); // TODO EL CORPUS
    cout << "Ha tardado " << getcputime() - aa << " segundos" << endl;

	// bool kCasosEspeciales = true, kpasarAminusculas = true;

	// list<string> lt1, lt2, lt3;

	// Tokenizador a("[]# ", kCasosEspeciales, kpasarAminusculas);
    // a.PasarAminuscSinAcentos(false);
	// a.DelimitadoresPalabra(" _");
    // a.Tokenizar("MS#DOS 10 España Éspáñé OS_2 [high low]", lt3);
    // imprimirListaSTL(lt3);
}