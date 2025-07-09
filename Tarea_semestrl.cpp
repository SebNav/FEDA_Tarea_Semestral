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


//Estructura User que contiene la informacion de un usuario de Twitter
struct User{

    long long ID; 
    string username;
    int n_tweets;
    int follower_count;
    int friends_count;
    vector<string> friends;
    vector<string> followers;
    vector<float> political_index; //Vector que contendra el indice politico de cada usuario
    float PageRank; //Indice de influencia de cada nodo basado en el algoritmo de pageRank google
    

    size_t Size_user() const{
        size_t tamaño = 0;
        tamaño += sizeof(ID); // ID
        tamaño += sizeof(n_tweets);
        tamaño += sizeof(follower_count);
        tamaño += sizeof(friends_count);
        tamaño += sizeof(PageRank);
        tamaño += sizeof(username) +  username.capacity();
        for(const string& f : followers)
        {
            tamaño += sizeof(f) + f.capacity();
        }
        for(const string& f : friends)
        {
            tamaño += sizeof(f) + f.capacity();
        }
        tamaño += sizeof(political_index) + (political_index.capacity()*sizeof(float));
        
        return tamaño;
    };
};

//Clase que representa un grafo dirigado de usuarios de twitters
class Grafo
{
private:

    unordered_map<string,User> nodos; //Undordered map que es la base del grafo, donde la key sera el username del grafo y se guardaran en los valores la variable Usuario
    multiset<pair<int,string>> influyentes; //Variable del tipo multiset que guardara el Top 10 de usuarios mas influyentes (Más followers)
    multiset<pair<int,string>> influenciables; //Variable del tipo multiset que guardara el Top 10 de usuarios mas influyenciables (Más seguidos)
    vector<string>   keys; // Vector que guardara todos los keys, este vector simplemente se utilzara para obtener keys de forma aletoria


    //Proceso de visita para la implementacion recursiva de DFS
    //Si pre_order es Verdadero se reporta el recorrido del DFS en forma pre-order de lo contrario en post-order
    void DFS_visit(const string& u,unordered_set<string>& visitados,vector<string>& resultado, bool IN_degree,bool Pre_order) const{
        
        visitados.insert(u); //Se marca que el nodo ha sido visitado
        vector<string> vecinos;
        //Si pre-oder, se agrega el nodo al resultado antes de visitar vecinos
        if (Pre_order)
        {
            resultado.push_back(u);
        }

        //Selecciona vecinos de entrada (followers) o salida (seguidos)
        if (IN_degree) vecinos = vecinos_in(u);
        else vecinos = vecinos_out(u);

        //Se visita recurrente todos los vecinos no visitados
        for (const string& n : vecinos)
        {
            if (visitados.count(n)==0)
            {
                DFS_visit(n,visitados,resultado,IN_degree,Pre_order);
            }
        }
        //Si es post-order, se agrega el nodo al resultado despues de visitar todos los vecinos
        if (!Pre_order) resultado.push_back(u);

        return;
    }

public:
    //Constructor del grafo a patir de los archivos csv con la informacion del usuario y las conecciones del grafo
    Grafo(const string& csv_users,const string& csv_connection){

        //Se intentan abrir los archivos de usuarios y conecciones
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

        //Se lee cada linea del archivo 
        while(getline(archivo_users,linea))
        {
            stringstream ss(linea);
            string info;
            vector<string> fila;
            //Se separa las sentencias si encuentra un delimitador ';'
            while (getline(ss,info,';'))
            {
                fila.push_back(info);
            }
            //Se crea la variable Ususario actual y se guaran la informacion que esta presente en el archivo leido
            User Usuario_actual;

            Usuario_actual.ID = stoll(fila[0]);
            Usuario_actual.username = fila[1];
            Usuario_actual.n_tweets = stoll(fila[2]);
            Usuario_actual.friends_count = 0;
            Usuario_actual.follower_count = 0;
            
            //Se añade al Unordered_map el usuario usando su username como key y sus informacion como valores
            nodos.insert({Usuario_actual.username,Usuario_actual});
            
        }
        
        archivo_users.close(); //Se cierra el archivo una vez que se termino la lectura de todas las lineas

        //Se lee cada linea del archivo connections y se guarda en los amigos y followers, la respectiva informaion de usuario
        while(getline(archivo_connect,linea)){

            stringstream ss(linea);
            string followee,follower;
            //Se leen y guardan de cada linea
            getline(ss, followee, ';');
            getline(ss, follower);
            //cout << followee<<endl;
            //Debido a que se tiene un grafo dirigo, donde la información del archivo muestra quien sigue a que usuario
            //Tambien podemos usar esta informacion para saber quienes son los followers de que usuario
            //Para ello se busca el followee para saber si se añadio en la etapa anterio y se guardar el follower en la variable friends indicado que este usuario sige a X
            //Ademas busamos el followers y añadimos a el followee como seguidor(followers) del usuario
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
        archivo_connect.close();//Se cierra el archivo

        //Aqui se leen todos los keys y se calcula la cantidad de friends y foloowers de cada usuario a partir del tamaño del vector de friends y followers
        //Ademas se calcula al mismo tiempos los mas influyentes
        for(auto& pair : nodos)
        {
            pair.second.friends_count   = pair.second.friends.size(); 
            pair.second.follower_count  = pair.second.followers.size();
            pair.second.PageRank        = 1.0/nodos.size(); //Inicalizacion del PageRank de cada usuario
            pair.second.political_index = {0.0,0.0,0.0,0.0}; //Inicializacion del indice politico de cada usuario
            keys.push_back(pair.first); //Se añade el nombre de usuario al vector de keys
            //Mientras no se tengan 10 datos simplemente se guardan el usuario y su numero de seguidores y friends
            //Se guarda primero en el par la cantidad de friends/followers ya que el multiset ordenara a partir de esta variable first
            if (influyentes.size() < 10) 
            {
                influyentes.insert({pair.second.followers.size(),pair.first});
                influenciables.insert({pair.second.friends.size(),pair.first});
            }

            //Si el primero valor del set, es decir el con menor cantidad de followers tiene menor cantidad que el nuevo nodo a observado se elimina y añade el nuevo
            else{
                if (influyentes.begin()->first < pair.second.followers.size())
                {
                    influyentes.erase(influyentes.begin());
                    influyentes.insert({pair.second.followers.size(),pair.first});
                }

                if (influenciables.begin()->first < pair.second.friends.size())
                {
                    influenciables.erase(influenciables.begin());
                    influenciables.insert({pair.second.friends.size(),pair.first});
                }
            }
            
        }
        
    }
    
    ~Grafo() {};

    //Funcion publica para imprimir un usuario y su información asociada
    void imprimirUsuario(const string& username) const {
        
        //Se busca si existe el usuario
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
            cout << "Inlfuencia (PageRank) : " << usuario.PageRank << endl;
            cout << "Siguiendo :";
            for(string amigos : usuario.friends){
                cout << " " << amigos << " ,";}
            cout << endl;
            cout << "Followers :";
            for(string seguidores : usuario.followers){
                cout << " " << seguidores << " ,";}
            cout << endl;
            cout << "Indice Politico [I, C, L, D]: [" << usuario.political_index[0] << "," << usuario.political_index[1]<<"," << usuario.political_index[2]<<"," << usuario.political_index[3] << "]" <<  endl;
            cout << "------------------------------------------" << endl;
        } else {
            // Si it == nodos.end(), el usuario no fue encontrado en el mapa.
            cout << "--> Usuario con username '" << username << "' no encontrado en el grafo." << endl;
            }
        }

    //Funcion que muestra el Top 10 de usuarios mas influyentes de forma descendiente 
    void TopInfluyentes() const {

            cout << "**Raking top 10 cuentas mas influyentes**"<<endl;
            int rank = 1;
            for (auto it = influyentes.rbegin(); it != influyentes.rend();++it,++rank)
            {


                cout << "Rank " << rank << ": " << "'" << it->second << "' ," << it->first 
                << " PageRank: " << nodos.at(it->second).PageRank << ", Indice Politico [I, C, L, D]: [" << nodos.at(it->second).political_index[0] <<  "," << nodos.at(it->second).political_index[1] <<"," << nodos.at(it->second).political_index[2] <<"," << nodos.at(it->second).political_index[3] << "]" <<  endl;
            }
        }
    //Funcion que muestra el Top 10 de usuarios mas influenciables de forma descendiente
    void TopInfluenciables() const {

        cout << "**Raking top 10 cuentas mas influenciables**"<<endl;
        int rank = 1;
        for (auto it = influenciables.rbegin(); it != influenciables.rend();++it,++rank)
        {
                cout << "Rank " << rank << ": " << "'" << it->second << "' ," << it->first 
                << " PageRank: " << nodos.at(it->second).PageRank << ", Indice Politico [I, C, L, D]: [" << nodos.at(it->second).political_index[0] <<  "," << nodos.at(it->second).political_index[1] <<"," << nodos.at(it->second).political_index[2] <<"," << nodos.at(it->second).political_index[3] << "]" <<  endl;
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
    //Implementacion de recursiva de DFS a partir de los datos los vecinos in o out dependiendo de la opcion colocado,
    // Si IN_degree es false, se obtiene el DFS normal (vecinos_out)
    // Si IN_degree es true, se obtiene el DFS del grafo inverso (vecinos_in)
    //Si Pre_order el resultado se reporta en forma pre order, sino se reporta en postorder
    vector<string> DFS(const string& username,const bool IN_degree,bool Pre_order) const{

        //Si nodos.count(username) = 0 significa que no existe el usuario en el mapa
        if(nodos.count(username) == 0){
            cout << username << " no es parte del grafo!!!!" << endl;
            return {};
        }

        vector<string> resultados;
        unordered_set<string> visitados;
        
        DFS_visit(username,visitados,resultados,IN_degree,Pre_order);
        
        return resultados;
    }
    
    //Funcion que implementa una busqueda BFS a partir de un nodo en particular
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
        visitados.clear(); //Se limpia la variable visitados para el segundo DFS que se realizara

        //DFS del grafo transpuesto (es decir In-order) sigiendo el orden de la pila
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


    }

    //Funcioón para impirmir aletoriamente la informacion de x cantidad de sujetos del grafo
    void RandomPrint(const int& cantidad,const int& random_state) const{

        
        default_random_engine engine(random_state);
        uniform_int_distribution<int> distribution(0,keys.size()-1);
        for (size_t i = 0; i < cantidad; i++)
        {
            int ran_index = distribution(engine);
            string random_key = keys[ran_index];   
            imprimirUsuario(random_key);
        }
        
        return;
        
    }
    
    //Funcion que calcula el PageRank lo que seria el nivel de inflencia de cada nodo a partir del algoritmo PageRank de google
    void PageRanking(const int& iterations) {

        unordered_map<string,float> next_PageRank; //Mapa para almacenar los nuevos valores del PageRank
        float PageRank_value;
        float d = 0.85; //Factor de amortiguacion
        float suma_votos;//Sumatoria de votos del nodo
        

        cout << "Se inicia el proceso de calculo de PageRank para cada nodo del grafo para " << iterations << " cantidad de iteraciones" << endl;

        //proceso iterativo que modificia el PageRank de cada Nodo
        for (size_t i = 0; i < iterations; i++)
        {
            //cout << "**Iteracion " << i << "**" << endl; 
            //Se itera para cada nodo y se actualiza el page rank
            //Se itera para cada nodo del grafo y se calcula un nuevo valor de PageRank
            for (auto& pair: nodos)
            {
                //Calculo de la formula standar de PageRank
                suma_votos = 0;
                for (const string& key: vecinos_in(pair.first))
                {
                    suma_votos += nodos[key].PageRank/static_cast<float>(nodos[key].friends_count);
                    
                }
                //Se calcula el pagerank con la formula estandar
                PageRank_value = (1-d)/nodos.size() + d*suma_votos;
                //Se guarda en otra variable y despues se guarda debido a que si se guardara inmediatamente afectaria al calculo de futuros nodos que sigan a este sujeto
                //modificando el calculo de la variable
                next_PageRank.insert({pair.first,PageRank_value});
                
            }
            //Actualizacion del PageRank
            for (auto& pair: nodos)
            {
                //cout << next_PageRank[pair.first]<< endl;
                pair.second.PageRank = next_PageRank[pair.first];
            }
            //Se limpia la variable
            next_PageRank.clear();
            
        }
        

    }    
    
    //Algoritmo de propagacion que calcula a partir de nodos semillas la influecian politica utilizando el Influencia(Pagerank) de cada nodo
    void Tendencia_politica(const string& izquieda,const string& centro,const string& libertario,const string& derecha)
    {
        //Se inicializan los nodos semillas de los distintos partidos politicos
        nodos[izquieda].political_index   = {100.0,0,0,0};
        nodos[centro].political_index     = {0,100.0,0,0};
        nodos[libertario].political_index = {0,0,100.0,0};
        nodos[derecha].political_index    = {0,0,0,100.0};

        //Cantidad de iteraciones, al igual que pageRank a mayor cantidad de iteraciones los nodos convergen a un valor
        for (int i = 0; i < 50; ++i) {
            unordered_map<string, vector<float>> next_political_index;
            
            //Para cada nodo se actualiza su valor
            for (const auto& pair : nodos) {


                // Si es un nodo semilla, su vector no cambia
                if (pair.first == izquieda || pair.first == centro || pair.first == libertario || pair.first ==derecha) {

                    next_political_index[pair.first] = pair.second.political_index;
                    continue;
                }

                // Si no es semilla, calcular su nuevo vector
                vector<float> suma_ponderada_vector(4, 0.0);
                float suma_de_pesos = 0.0;//Suma de los pesos de todos los vecinos de salida para posteriormente ponderarlo y obtener valors entre 0 y 100
                //Se itera para cada usuario que sigue el nodo u para saber su indice polico
                //"Dime a quien sigues y te dire tu indice politico...., Ademas de más informacion sobre el grafo ajjaja"
                for (const string& v : vecinos_out(pair.first)) {
                    //Se obtiene el indice politico para cada categoria politica
                    for (int j = 0; j < 4; ++j) {
                        suma_ponderada_vector[j] += nodos[v].political_index[j] * nodos[v].PageRank;
                    }
                    suma_de_pesos += nodos[v].PageRank; 
                }

                vector<float> nuevo_vector(4, 0.0);
                //Ponderacion de indice politico
                if (suma_de_pesos > 0) {
                    for (int j = 0; j < 4; ++j) {
                        nuevo_vector[j] = suma_ponderada_vector[j] / suma_de_pesos;
                    }
                }
                
                // Normalización para que sume 100%
                float total_score = accumulate(nuevo_vector.begin(), nuevo_vector.end(), 0.0f);
                if (total_score > 0) {
                    for (int j = 0; j < 4; ++j) {
                        nuevo_vector[j] = (nuevo_vector[j] / total_score) * 100.0f;
                    }
                }
                next_political_index[pair.first] = nuevo_vector;
            }

            // Actualizar todos los vectores al final de la iteración
            for (auto& pair : nodos) {
                pair.second.political_index = next_political_index[pair.first];
            }
        }
        cout << "Cálculo de tendencias Politicas finalizado finalizado." << endl;
    }

    size_t Size_grafoB() const{

        size_t size_graph = 0;

        //Size del unordered_map nodos
        for (const auto& pair : nodos)
        {
            size_graph += sizeof(pair.first) + pair.first.capacity(); //Tamaño de la key
            size_graph += pair.second.Size_user();
        }
        //Size del multiset influyentes
        for (const auto& pair: influyentes)
        {
            size_graph += sizeof(pair.first);
            size_graph += sizeof(pair.second) + pair.second.capacity();
        }
        //Size del multiset influenciables
        for (const auto& pair: influenciables)
        {
            size_graph += sizeof(pair.first);
            size_graph += sizeof(pair.second) + pair.second.capacity();
        }
        //Size keys
        for (const string& key : keys)
        {
            size_graph += sizeof(key) + key.capacity();
        }
        
        return size_graph;
        
    }

    size_t Size_grafoKB() const{

        size_t tamañoKB = Size_grafoB()/1000;

        return tamañoKB;
    }


};



void print_string_vector(const vector<string>& s,const string& frase_previa) {

    cout << "*******" <<frase_previa <<"*******"<< endl;
    for (size_t i = 0; i < s.size(); ++i) {
        cout << s[i] << endl;
    }
    
    return;
}

int main(){

    Grafo mi_grafo("twitter_users.csv","twitter_connections.csv");
    string izquierda = "Cooperativa";
    string centro = "latercera";
    string libertario = "elmostrador";
    string derecha = "soyvaldiviacl";


/*     mi_grafo.RandomPrint(3,42);
    mi_grafo.imprimirUsuario("Cooperativa"); */
    mi_grafo.PageRanking(100);
    mi_grafo.Tendencia_politica(izquierda,centro,libertario,derecha);

    size_t tamaño_byte =mi_grafo.Size_grafoB();
    cout << "Tamaño total de la clase grafo " << tamaño_byte << endl;
    size_t tamaño_Kbyte =mi_grafo.Size_grafoKB();
    cout << "Tamaño total de la clase grafo " << tamaño_Kbyte << endl;




/*     mi_grafo.imprimirUsuario("jcmr2009");
    mi_grafo.imprimirUsuario("pachidiaze");
    mi_grafo.imprimirUsuario("Juventud_2021CL");
    mi_grafo.RandomPrint(10,19);
    mi_grafo.imprimirUsuario("Keutdertapia"); */
/*     mi_grafo.RandomPrint(3,42);
    mi_grafo.imprimirUsuario("Cooperativa");


    

    mi_grafo.imprimirUsuario("Cooperativa");
    mi_grafo.imprimirUsuario("latercera");
    mi_grafo.imprimirUsuario("elmostrador");
    mi_grafo.imprimirUsuario("CherieA81311446");
    mi_grafo.RandomPrint(5,10);
 */

    //mi_grafo.TopInfluenciables();
    //mi_grafo.TopInfluyentes();

/*      vector<string> vecinos_in = mi_grafo.vecinos_in("patriciorosas");
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

 */
    return 0;
}

