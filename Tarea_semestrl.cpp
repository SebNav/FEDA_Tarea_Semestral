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
#include<queue>


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
    multiset<pair<int,string>> influyentes; //más seguidores
    multiset<pair<int,string>> influenciables; //más followers

    //Proceso de visita para la implementacion recursiva de DFS, si pre_order es Verdadero se reporta el recorrido del BFS en forma pre-order de lo contrario en post-order
    void DFS_visit(const string& u,unordered_set<string>& visitados,vector<string>& resultado, bool IN_degree,bool Pre_order) const{
        
        visitados.insert(u);
        vector<string> vecinos;
        if (Pre_order)
        {
            resultado.push_back(u);
        }

        if (IN_degree) vecinos = vecinos_in(u);
        else vecinos = vecinos_out(u);

        for (const string& n : vecinos)
        {
            if (visitados.count(n)==0)
            {
                DFS_visit(n,visitados,resultado,IN_degree,Pre_order);
            }
        }
        if (!Pre_order) resultado.push_back(u);

        return;
    }

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
    //Implementacion de DFS a partir de los datos los vecinos in o out dependiendo de la opcion colocado,
    // Si IN_degree es false, se obtiene el DFS normal (vecinos_out)
    // Si IN_degree es true, se obtiene el DFS del grafo inverso (vecinos_in)
    //Si Pre_order el resultado se reporta en forma pre order, sino se reporta en postorder
    vector<string> DFS(const string& username,const bool IN_degree,bool Pre_order) const{

        if(nodos.count(username) == 0){
            cout << username << " no es parte del grafo!!!!" << endl;
            return {};
        }

        vector<string> resultados;
        unordered_set<string> visitados;
        
        DFS_visit(username,visitados,resultados,IN_degree,Pre_order);
        
        return resultados;
    }
    
    vector<string> BFS(const string& username,const bool IN_degree) const{
        auto it = nodos.find(username);
        if (it != nodos.end())
        {
            vector<string> resultado; //Vector donde se almacenara el recorrido del dfs
            unordered_set<string> visitados; //A diferencia de unordered map que usa key y valor este solo usa una clave, por lo que hace mas facil e eficiente saber si visitamos o no ese nodo
            queue<string> cola; 
            
            cola.push(username);
            visitados.insert(username);
            while (!cola.empty())
            {
                string u = cola.front();
                cola.pop();
                resultado.push_back(u);

                vector<string> vecinos;
                if (IN_degree) vecinos = vecinos_in(u);
                else vecinos = vecinos_out(u);

                for (const string& n : vecinos) {
                    if (visitados.count(n)==0) {
                        visitados.insert(n);
                        cola.push(n);
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
    //Implementacion de funcion que calcula las componentes fuertemente conexas del grafo utilizando el algoritmo de Kosaraju
    vector<vector<string>> CFC() const{

        vector<string> orden; //Orden de visita para el grafo invertido
        unordered_set<string> visitados;

        //Se recorren todos los nodos del grafo para obtener el orden de finalizacion mediante post orden de DFS
        for (const auto& par : nodos)
        {
            vector<string> DFS_out;
            if(visitados.count(par.first)==0) //Si el nodo del grafo ya no ha sido visitado, se inicia un DFS desde este nodo
            {
                DFS_visit(par.first,visitados,orden,false,false);
            }
        }
        //Se pasa el orden a una pila para tener el orden necesario para relizar el DFS en el grafo inverso
        stack<string> pila;
        for (const string& o: orden)
        {
            pila.push(o);
        }

        vector<vector<string>> componentes_FC;
        visitados.clear(); //Se limpia visitados porq se volvera a realizar DFS esta vez para obtener las componestes

        while (!pila.empty())
        {
            string nodo = pila.top();
            pila.pop();
            vector<string> componenteFC;
            if(visitados.count(nodo) == 0) //Si el nodo no ha sido visitado
            {
                DFS_visit(nodo,visitados,componenteFC,true,true);
                componentes_FC.push_back(componenteFC);
            }
        }
        return componentes_FC;
    }};




void print_string_vector(const vector<string>& s,const string& frase_previa) {

    cout << "*******" <<frase_previa <<"*******"<< endl;
    for (size_t i = 0; i < s.size(); ++i) {
        cout << s[i] << endl;
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
    //print_string_vector(vecinos_in,"vecinos in/followers de patriciorosas");

    vector<string> vecinos_out = mi_grafo.vecinos_out("patriciorosas");
    //print_string_vector(vecinos_out,"vecinos out/seguidos de patriciorosas");

    vector<string> DFS_a = mi_grafo.DFS("patriciorosas",false,true);

    print_string_vector(DFS_a,"DFS desde 'c_flores_c' out degree, con pre orden");

    vector<string> DFS_b = mi_grafo.DFS("patriciorosas",false,false);
    print_string_vector(DFS_b,"DFS desde 'c_flores_c' out degree, con post orden");

    vector<vector<string>> CFC = mi_grafo.CFC();
    cout << "El grafo tiene en total " << CFC.size() << "componentes fuertemente conexas" <<endl;
    int i = 1;
    for(const vector<string>& componente : CFC){
        if(componente.size() > 1) cout << "CFC " << i++ << " N° nodos : "  << componente.size() << endl;

    }



    return 0;
}

