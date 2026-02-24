#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <fstream>
#include "../include/tokenizador.h"
#include <string>
#include <unistd.h>
using namespace std;

Tokenizador::Tokenizador (const string& delimitadoresPalabra, const bool& kcasosEspeciales, const bool& minuscSinAcentos) {
    DelimitadoresPalabra(delimitadoresPalabra);
    casosEspeciales = kcasosEspeciales;
    pasarAminuscSinAcentos = minuscSinAcentos;
}

Tokenizador::Tokenizador (const Tokenizador& t) {
    delimiters = t.delimiters;
    casosEspeciales = t.casosEspeciales;
    pasarAminuscSinAcentos = t.pasarAminuscSinAcentos;
}

Tokenizador::Tokenizador () {
    delimiters = ",;:.-/+*\\ '\"{}[]()<>¡!¿?&#=\t@";
    casosEspeciales = true;
    pasarAminuscSinAcentos = false;
}

Tokenizador::~Tokenizador () {
    delimiters = "";
}   

Tokenizador& Tokenizador::operator= (const Tokenizador& t) {
    if (this != &t) {
        delimiters = t.delimiters;
        casosEspeciales = t.casosEspeciales;
        pasarAminuscSinAcentos = t.pasarAminuscSinAcentos;
    }
    return *this;
}

void Tokenizador::Tokenizar (const string& str, list<string>& tokens) const {

    string delimiters = this->delimiters; // copia local
    if (casosEspeciales) {
        if (delimiters.find(' ')  == string::npos) delimiters += ' ';
        if (delimiters.find('\n') == string::npos) delimiters += '\n';
        if (delimiters.find('\r') == string::npos) delimiters += '\r';
        if (delimiters.find('\t') == string::npos) delimiters += '\t';
    }


    //añadir cosa para meter espacio a delimiters aquí

    string delimitersURL="";//Esto lo tendré que hacer para que se haga sólo una vez y no con cada instania de Tokenizar
    string excepcionesURL="_:/.?&-=#@";
    for (char c : delimiters) {

        if (excepcionesURL.find(c) == string::npos) {
            delimitersURL += c;
        }
    }
    string delimitersNUM="";//Esto lo tendré que hacer para que se haga sólo una vez y no con cada instania de Tokenizar
    string excepcionesNUM=".,";
    for (char c : delimiters) {

        if (excepcionesNUM.find(c) == string::npos) {
            delimitersNUM += c;
        }
    }

    string delimitersEMAIL="";//Esto lo tendré que hacer para que se haga sólo una vez y no con cada instania de Tokenizar
    string excepcionesEMAIL=".-_";
    for (char c : delimiters) {

        if (excepcionesEMAIL.find(c) == string::npos) {
            delimitersEMAIL += c;
        }
    }

    string delimitersACRON="";//Esto lo tendré que hacer para que se haga sólo una vez y no con cada instania de Tokenizar
    string excepcionesACRON=".";
    for (char c : delimiters) {

        if (excepcionesACRON.find(c) == string::npos) {
            delimitersACRON += c;
        }
    }

    string delimitersGUION="";//Esto lo tendré que hacer para que se haga sólo una vez y no con cada instania de Tokenizar
    string excepcionesGUION="-";
    for (char c : delimiters) {

        if (excepcionesGUION.find(c) == string::npos) {
            delimitersGUION += c;
        }
    }

    bool esDelimPunto = (delimiters.find('.') != string::npos);
    bool esDelimComa  = (delimiters.find(',') != string::npos);
    bool esDelimArroba = (delimiters.find('@') != string::npos);
    bool esDelimGuion = (delimiters.find('-') != string::npos);



    tokens.clear();

    string txt = str;

    if (pasarAminuscSinAcentos) {
        txt = Normalizar(txt);
    }

    //string delimiters = delimiters + "\n\r"; esto está fatal
    //if (casosEspeciales && delimiters.find(' ') == string::npos) delimiters += ' ';

    string::size_type lastPos = txt.find_first_not_of(delimiters,0);

    if(casosEspeciales)
    {
        while(string::npos != lastPos)
        {

            //URLs
            if (txt.compare(lastPos, 4, "ftp:") == 0 || txt.compare(lastPos, 5, "http:") == 0 || txt.compare(lastPos, 6, "https:") == 0) 
            {
                string::size_type fin = txt.find_first_of(delimitersURL, lastPos);
                if (fin == string::npos) {fin = txt.size();}
                if (txt[fin-1] !=':'){
                    string url = txt.substr(lastPos, fin - lastPos);
                    tokens.push_back(url);
                    lastPos = txt.find_first_not_of(delimiters, fin);
                }
                else
                {
                    string::size_type pos = txt.find_first_of(delimiters, lastPos);
                    tokens.push_back(txt.substr(lastPos, pos - lastPos));
                
                    // Avanzamos lastPos a la siguiente palabra
                    if (pos == string::npos){
                        lastPos = string::npos;
                    }
                    else{ 
                        lastPos = txt.find_first_not_of(delimiters, pos);
                    }
                }
            }
            //DECIMALES
            else if ((esDelimPunto || esDelimComa) && 
                (isdigit(txt[lastPos]) || ((txt[lastPos]=='.' || txt[lastPos]==',') 
                && lastPos+1<txt.length() && isdigit(txt[lastPos+1]))))
            {
                bool esNumero = true;
                string token="";
                string::size_type i = lastPos;
                if (lastPos > 0 &&
                    (txt[lastPos-1]=='.' || txt[lastPos-1]==','))
                {
                    token += '0';
                    token += txt[lastPos-1];  // añadir el separador
                }

                while(i<txt.size())
                {
                    unsigned char c = txt[i];
                    if(isdigit(c) ||
                            (c == '.' && esDelimPunto) ||
                            (c == ',' && esDelimComa))
                    {
                        token+=c;
                        i++;
                    }
                    else
                    {
                        break;
                    }
                }
                while (!token.empty() && (token.back()=='.' || token.back()==',')){
                    token.pop_back();
                }
                if (!token.empty()){
                    tokens.push_back(token);
                }
                lastPos = txt.find_first_not_of(delimiters, i);
            }//EMAILS
            else if(esDelimArroba && (txt.find('@', lastPos) <= txt.find_first_of(delimiters + " \t\n\r", lastPos)) &&  txt.find('@', lastPos) != lastPos && txt.find('@', lastPos) != string::npos)
            {
                string::size_type posArroba = txt.find('@', lastPos);

                string::size_type i = posArroba + 1;
                bool emailValido = true;
                // REGLA: Debe contener algo después de la @ y que no sea delimitador ni espacio
                bool tieneTextoDespues = (i < txt.size() && 
                    txt[i] != ' ' && txt[i] != '\t' && txt[i] != '\n' && txt[i] != '\r' && 
                    delimiters.find(txt[i]) == string::npos);

                if(tieneTextoDespues)
                {
                    while(i<txt.size())
                    {
                        unsigned char c = txt[i];

                        if(c=='@')
                        {
                            emailValido=false;
                            break;
                        }

                        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || delimitersEMAIL.find(c) !=string::npos){
                            break;
                        }
                        else if(c == '.' || c=='-' || c=='_')
                        {
                            unsigned char prev = txt[i-1];
                            char next = (i + 1 < txt.size()) ? txt[i+1] : '\0';
                            if (delimiters.find(prev) == string::npos && prev != ' ' && prev != '.' && prev != '-' && prev != '_' &&
                                next != '\0' && delimiters.find(next) == string::npos && next != ' '){
                                i++;
                            }
                            else{ 
                                break;
                            }
                        }
                        else//Carácter normal
                        {
                            i++;
                        }


                    }
                    if (emailValido) {
                        tokens.push_back(txt.substr(lastPos, i - lastPos));
                        lastPos = txt.find_first_not_of(delimiters + " \t\n\r", i);
                    } else {
                        // Segundo @ encontrado 
                        if (posArroba > lastPos) {
                            tokens.push_back(txt.substr(lastPos, posArroba - lastPos));
                        }
                        lastPos = posArroba + 1;
                        continue;
                    }
                }
                else{//Fake email como marcos@
                    if (posArroba > lastPos){
                        tokens.push_back(txt.substr(lastPos, posArroba - lastPos));
                        lastPos = txt.find_first_not_of(delimiters, posArroba + 1);
                    }
                    else
                    {//solo @, saltar
                        lastPos = txt.find_first_not_of(delimiters, posArroba + 1);
                    }
                }
            }//ACRÓNIMOS
            else if(esDelimPunto && txt.find('.', lastPos) < txt.find_first_of(delimitersACRON + " \t\n\r", lastPos))
            {
                string::size_type i = lastPos;

                while(i<txt.size())
                {
                    unsigned char c = txt[i];
                    if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || delimitersACRON.find(c) !=string::npos){
                        break;
                    }
                    else if(c == '.')
                    {
                        unsigned char prev = txt[i-1];
                        char next = (i + 1 < txt.size()) ? txt[i+1] : '\0';
                        if (delimiters.find(prev) == string::npos && prev != '.' &&
                            next != '\0' && delimiters.find(next) == string::npos && next != '.')
                            i++;
                        else break;
                    }
                    
                    else//Carácter normal
                    {
                        i++;
                    }

                }
                if (i > lastPos)
                    tokens.push_back(txt.substr(lastPos, i - lastPos));
    
                lastPos = (i >= txt.size()) ? string::npos : txt.find_first_not_of(delimiters, i);
            }//GUIONES
            else if (esDelimGuion &&
                txt.find('-', lastPos) < txt.find_first_of(delimitersGUION + " \t\n\r", lastPos) &&
                txt.find('-', lastPos) != lastPos && txt.find('-', lastPos) != string::npos)
            {
                string::size_type i = lastPos;

                while (i < txt.size())
                {
                    unsigned char c = txt[i];
                    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') break;
                    if (c == '-')
                    {
                        char next = (i + 1 < txt.size()) ? txt[i+1] : '\0';
                        // Dos guiones seguidos o guion al final: cortar antes
                        if (next == '-' || next == '\0' || next == ' ' || delimiters.find(next) != string::npos) break;
                        i++;
                    }
                    else if (delimiters.find(c) != string::npos) {break;}
                    else {i++;}
                }

                if (i > lastPos)
                    tokens.push_back(txt.substr(lastPos, i - lastPos));

                lastPos = (i >= txt.size()) ? string::npos : txt.find_first_not_of(delimiters, i);
            }
            else
            {//NO es ningun caso especial
                string::size_type pos = txt.find_first_of(delimiters, lastPos);
                tokens.push_back(txt.substr(lastPos, pos - lastPos));
                
                // Avanzamos lastPos a la siguiente palabra
                if (pos == string::npos){
                    lastPos = string::npos;
                }
                else{ 
                    lastPos = txt.find_first_not_of(delimiters, pos);
                }
            }
            }
            
    }
    else
    {
        string::size_type pos = txt.find_first_of(delimiters,lastPos);

        while(string::npos != lastPos){
            tokens.push_back(txt.substr(lastPos, pos - lastPos));
            lastPos = txt.find_first_not_of(delimiters, pos);
            pos = txt.find_first_of(delimiters, lastPos);
        }
    }
}

string Tokenizador::Normalizar(const string& str) const {
    string resultado = str;
    for (size_t i = 0; i < resultado.length(); i++) {
        unsigned char c = resultado[i];
        if (c >= 'A' && c <= 'Z')
            resultado[i] = tolower(c);
        else if (c==193||c==192||c==194||c==196||c==225||c==224||c==226||c==228)
            resultado[i] = 'a';
        else if (c==201||c==200||c==202||c==203||c==233||c==232||c==234||c==235)
            resultado[i] = 'e';
        else if (c==205||c==204||c==206||c==207||c==237||c==236||c==238||c==239)
            resultado[i] = 'i';
        else if (c==211||c==210||c==212||c==214||c==243||c==242||c==244||c==246)
            resultado[i] = 'o';
        else if (c==218||c==217||c==219||c==220||c==250||c==249||c==251||c==252)
            resultado[i] = 'u';
        else if (c==209||c==241)
            resultado[i] = 241; // Ñ->ñ
    }
    return resultado;
}
bool Tokenizador::Tokenizar (const string& NomFichEntr, const string& NomFichSal) const {
    ifstream i;
    ofstream f;
    string cadena;
    list<string> tokens;
    tokens.clear();

    i.open(NomFichEntr.c_str());
    if(!i) {
        cerr << "ERROR: No existe el archivo:" << NomFichEntr << endl;
        return false;
    }
    else
    {
        while(!i.eof())
        {
            cadena="";
            getline(i, cadena);
            if(cadena.length()!=0)
            {
                Tokenizar(cadena, tokens);
            }
        }
    }
    i.close();
    f.open(NomFichSal.c_str());
    list<string>::iterator itS;
    for(itS= tokens.begin();itS!= tokens.end();itS++)
    {
        f << (*itS) << endl;
    }
    f.close();
    return true;
}

bool Tokenizador::Tokenizar(const string& i) const {
    // El enunciado dice: "guardando la salida en un fichero de nombre i 
    // añadiéndole extensión .tk (sin eliminar previamente la extensión de i)"
    string nombreSalida = i + ".tk";
    
    // Invocamos a la otra función miembro para reutilizar la lógica de apertura
    // de ficheros y manejo de errores.
    return Tokenizar(i, nombreSalida);
}

bool Tokenizador::TokenizarListaFicheros(const string& i) const {
    ifstream ficheroLista;
    string nombreFicheroIndividual;
    bool todoCorrecto = true;

    ficheroLista.open(i.c_str());

    if (!ficheroLista) {
        cerr << "ERROR: No existe el archivo de lista: " << i << endl;
        return false;
    }

    // Leemos cada línea del fichero i, que contiene un nombre de fichero a tokenizar
    while (getline(ficheroLista, nombreFicheroIndividual)) {
        // El enunciado dice que no se debe interrumpir la ejecución si un archivo no existe,
        // pero el valor de retorno final debe ser false si alguno falló.
        if (!nombreFicheroIndividual.empty()) {
            // Llamamos a la versión de Tokenizar que añade .tk automáticamente
            if (!Tokenizar(nombreFicheroIndividual)) {
                todoCorrecto = false; 
                // El error específico de cada fichero ya lo lanza el método Tokenizar(string) a cerr
            }
        }
    }

    ficheroLista.close();
    return todoCorrecto;
}
bool Tokenizador::TokenizarDirectorio (const string& dirAIndexar) const {
    struct stat dir;
    // Compruebo la existencia del directorio
    int err=stat(dirAIndexar.c_str(), &dir);
    if(err==-1 || !S_ISDIR(dir.st_mode)){
        cerr << "ERROR: No existe el directorio: " << dirAIndexar << endl; // <--- OBLIGATORIO
        return false;
    }
    else {
        // Hago una lista en un fichero con find>fich
        string cmd="find "+dirAIndexar+" -follow |sort > .lista_fich";
        system(cmd.c_str());
        return TokenizarListaFicheros(".lista_fich");
    }
}

void Tokenizador::DelimitadoresPalabra(const string& nuevoDelimiters) {
    delimiters = "";
    for (char c : nuevoDelimiters) {
        if (delimiters.find(c) == string::npos) {
            delimiters += c;
        }
    }
}

void Tokenizador::AnyadirDelimitadoresPalabra(const string& nuevoDelimiters) {
    for (char c : nuevoDelimiters) {
        if (delimiters.find(c) == string::npos) {
            delimiters += c;
        }
    }
}

string Tokenizador::DelimitadoresPalabra() const
{
    return delimiters;
}

void Tokenizador::CasosEspeciales(const bool& nuevoCasosEspeciales)
{
    casosEspeciales=nuevoCasosEspeciales;
}

bool Tokenizador::CasosEspeciales()
{
    return casosEspeciales;
}

void Tokenizador::PasarAminuscSinAcentos(const bool& nuevoPasarAminuscSinAcentos)
{
    pasarAminuscSinAcentos=nuevoPasarAminuscSinAcentos;
}

bool Tokenizador::PasarAminuscSinAcentos()
{
    return pasarAminuscSinAcentos;
}


ostream& operator<<(ostream& os, const Tokenizador& t) {
    os << "DELIMITADORES: " << t.delimiters 
       << " TRATA CASOS ESPECIALES: " << t.casosEspeciales 
       << " PASAR A MINUSCULAS Y SIN ACENTOS: " << t.pasarAminuscSinAcentos;
    return os;
}