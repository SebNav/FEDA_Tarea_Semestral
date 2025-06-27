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
#include<set>
#include<unordered_map>
#include<unordered_set>


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

    unordered_map<string,User> nodos;
    multiset<pair<int_fast32_t,string>> influyentes; //más seguidores
    multiset<pair<int,string>> influenciables; //más followers



public:
    Grafo(const string& csv_users,const string& csv_connection){

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
            Usuario_actual.friends_count = 0;
            Usuario_actual.follower_count = 0;
            
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

        //Aqui se leen todos los keys y se calcula el valor de frinds y foloowers counts a partir del tamaño de vector de amigos y followers
        //Ademas se calcula al mismo tiempos los mas influyentes
        for(auto& pair : nodos)
        {
            pair.second.friends_count = pair.second.friends.size();
            pair.second.follower_count = pair.second.followers.size();

            if (influyentes.size() < 10)
            {
                influyentes.insert({pair.second.followers.size(),pair.first});
                influenciables.insert({pair.second.friends.size(),pair.first});
            }
            else{
                if (influyentes.begin()->first < pair.second.followers.size())//Si el primero del set, es decir el con menor cantidad de followers tiene menor cantidad que el nuevo nodo a obsercar se elimina y añade el nuevo
                {
                    influyentes.erase(influyentes.begin());
                    influyentes.insert({pair.second.followers.size(),pair.first});
                }

                if (influenciables.begin()->first < pair.second.friends.size())
                {
                    influenciables.erase(influenciables.begin());
                    influenciables.insert({pair.second.friends.size(),pair.first,});
                }
            }
        }
    }
    
    ~Grafo() {};


    void imprimirUsuario(const string& username) const {
        
        auto it = nodos.find(username);
        if (it != nodos.end()) {
            // it es un puntero a clave, valor)
            // it->second nos da acceso a User.
            const User& usuario = it->second; //Debido a que se usa const en la funcion, es necesario usar const aqui ya
            
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


    void TopInfluyentes() const {

            cout << "**Raking top 10 cuentas mas influyentes**"<<endl;
            int rank = 1;
            for (auto it = influyentes.rbegin(); it != influyentes.rend();++it,++rank)
            {
                cout << "Rank " << rank << ": " << "'" << it->second << "' ," << it->first << endl;  
            }
        }

    void TopInfluenciables() const {

        cout << "**Raking top 10 cuentas mas influenciables**"<<endl;
        int rank = 1;
        for (auto it = influenciables.rbegin(); it != influenciables.rend();++it,++rank)
        {
            cout << "Rank " << rank << ": " << "'" << it->second << "' ," << it->first << endl;  
        }


    }
    //Implementacion de funcion que retorna los vecinos de entrada o in-degree  de un nodo o followers de una cuenta
    vector<string> vecinos_in(const string& username) const{

        auto it = nodos.find(username);
        if(it != nodos.end())   return it->second.followers;   
        else { 
        cout << username << " no es parte del grafo!!!!" << endl;
        return {};
        } 
    }
    //Implementacion de funcion que retorna los vecinos de salida o out-degree  de un nodo o seguidos de una cuenta
    vector<string> vecinos_out(const string& username) const{

        auto it = nodos.find(username);
        if(it != nodos.end())   return it->second.friends;   
        else { 
        cout << username << " no es parte del grafo!!!!" << endl;
        return {};
        } 
    }
    //Implementacion de DFS a partir de los datos los vecinos in o out dependiendo de la opcion colocado, Si la variable IN_degree es verdadera se obtiene el DFS normal,
    //En caso contrario se obtiene el DBS del grafo inverso
    vector<string> DFS(const string& username,const bool IN_degree) const{
        auto it = nodos.find(username);
        if (it != nodos.end())
        {
            vector<string> resultado; //Vector donde se almacenara el recorrido del dfs
            unordered_set<string> visitados; //A diferencia de unordered map que usa key y valor este solo usa una clave, por lo que hace mas facil e eficiente saber si visitamos o no ese nodo
            stack<string> pila; 
            pila.push(username);

            while (!pila.empty())
            {
                string u = pila.top();
                pila.pop();
                if (visitados.count(u) == 0) //Si no existe ningun elemento en el unordered_set significa que no ha sido visitado ese nodo
                {

                    visitados.insert(u); //Lo guarda como visitado
                    resultado.push_back(u);

                    vector<string> vecinos;
                    if (IN_degree == true) //Busca el DFS de los vecinos que llegan, esto genera el DFS inverso del nodo
                    {
                        vecinos = vecinos_in(u);
                        for (const string& n : vecinos){
                            if(visitados.count(n) ==0) pila.push(n);
                        }
                    }
                    else{
                        vecinos = vecinos_out(u);
                        for (const string& n : vecinos){
                            if(visitados.count(n) ==0) pila.push(n);
                        }

                    }
                    
                    
                }
                

            }
            return resultado;
            
        }
        else { 
        cout << username << " no es parte del grafo!!!!" << endl;
        return {};
        }
        

    }
    

};


void print_string_vector(const vector<string>& s) {

    for (string u : s)
    {
        cout<< u << endl;
    }

    return;

}

int main(){

    Grafo mi_grafo("twitter_users.csv","twitter_connections.csv");

    
    mi_grafo.imprimirUsuario("Natanie27038290");
    cout << endl;
    mi_grafo.TopInfluenciables();
    mi_grafo.TopInfluyentes();

    vector<string> vecinos_in = mi_grafo.vecinos_in("patriciorosas");
    cout << "vecinos in/followers de patriciorosas" << endl;
    print_string_vector(vecinos_in);
    vector<string> vecinos_out = mi_grafo.vecinos_out("patriciorosas");
    cout << "vecinos out/seguidos de patriciorosas" << endl;
    print_string_vector(vecinos_out);

    vector<string> DFS_a = mi_grafo.DFS("patriciorosas",true);
    cout << "*****DFS_IN desde patriciorosas*****" << endl;
    print_string_vector(DFS_a);

    vector<string> DFS_b = mi_grafo.DFS("patriciorosas",false);
    cout << "*****DFS_out desde patriciorosas*****" << endl;
    //print_string_vector(DFS_b);

    
    return 0;
}

