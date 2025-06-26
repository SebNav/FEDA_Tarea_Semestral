#include<iostream>
#include<string>
#include<cmath>
#include<tuple>
#include<chrono>
#include<fstream>
#include<sstream>
#include<vector>
#include<list>
#include<random>
#include<map>
#include<algorithm>
#include<stack>


using namespace std;

struct User{

    long long ID;
    string username;
    int n_tweets;
    int follower_count;
    int friends_count;
    vector<string> friends;
    vector<string> followers;
    tuple<float,float,float,float> political_index;
    float presence;
    

};


class Grafo
{
private:

    map<string,User> nodos;

public:
    Grafo(string csv_users,string csv_connection){

        ifstream archivo_users(csv_users);
        if(!archivo_users.is_open())
        {
            cerr << "No se pudo abrir archivo " + csv_users << endl;
            exit(1);
        }

        ifstream archivo_connect(csv_connection);
        if(!archivo_connect.is_open())
        {
            cerr << "No se pudo abrir archivo " + csv_connection << endl;
            exit(1);
        }
        

        string linea;
        getline(archivo_users,linea);//No se leen los ID

        while(getline(archivo_users,linea))
        {
            stringstream ss(linea);
            string info;
            vector<string> fila;

            while (getline(ss,info,';'))
            {
                fila.push_back(info);
            }
            User Usuario_actual;

            Usuario_actual.ID = stoll(fila[0]);
            Usuario_actual.username = fila[1];
            Usuario_actual.n_tweets = stoll(fila[2]);
            Usuario_actual.friends_count = stoll(fila[3]);
            Usuario_actual.follower_count = stoll(fila[4]);
            
            nodos.insert({Usuario_actual.username,Usuario_actual});
        }

        archivo_users.close();

        while(getline(archivo_connect,linea)){

            stringstream ss(linea);
            string followee,follower;
            getline(ss, followee, ';');
            getline(ss, follower);
            cout << followee<<endl;
            //Se busca la cuenta del seguido y se añade su seguidor
            auto it_1 = nodos.find(followee);
            if (it_1 != nodos.end())
            {
                User& usuario_1 = it_1->second;
                usuario_1.followers.push_back(follower);
            }
        
            //Se busca la cuenta del seguidor y se añade su seguido
            auto it_2 = nodos.find(follower);
            if (it_2 != nodos.end())
            {
                User& usuario_2 = it_2->second;
                usuario_2.friends.push_back(followee);
            }
            
        }

        archivo_connect.close();
    }
    
    ~Grafo() {};


    void imprimirUsuario(const string& username) {
    auto it = nodos.find(username);

    if (it != nodos.end()) {
        // it es un puntero a clave, valor)
        // it->second nos da acceso a User.
        User& usuario = it->second;
        
        cout << "---Datos del Usuario: " << usuario.username << " ---" << endl;
        cout << "ID: " << usuario.ID << endl;
        cout << "Username: " << usuario.username << endl;
        cout << "N de Tweets: " << usuario.n_tweets << endl;
        cout << "N de Amigos (Friends): " << usuario.friends_count << endl;
        cout << "N de Seguidores (Followers): " << usuario.follower_count << endl;
        cout << "Siguiendo :";
        for(string amigos : usuario.friends){
            cout << " " << amigos << " ," << endl;}
        cout << endl;
        cout << "Followers :";
        for(string seguidores : usuario.followers){
            cout << " " << seguidores << " ," << endl;}
        cout << endl;
        cout << "Indice Politico : [" << get<0>(usuario.political_index) << "," << get<1>(usuario.political_index)<<"," << get<2>(usuario.political_index)<<"," << get<3>(usuario.political_index) << "]" <<  endl;
        cout << "------------------------------------------" << endl;
    } else {
        // Si it == nodos.end(), el usuario no fue encontrado en el mapa.
        cout << "--> Usuario con username '" << username << "' no encontrado en el grafo." << endl;
        }
    }
};


int main(){

    Grafo mi_grafo("twitter_users.csv","twitter_connections.csv");

    
    mi_grafo.imprimirUsuario("Cooperativa");
    cout << endl;

    return 0;
}

