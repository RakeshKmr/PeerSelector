#include <mysql++.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#define DB "peerselector" //database name
#define HOST "localhost" // so, where's your mysql server?
#define USERNAME "root" // a user granted access to the above database?
#define PASSWORD "CareNet2011" // enter the password for the above user. If there's no password, leave it as it is...
using namespace std;
int length = 0;
struct ipPort_s
{
    string ipAddress;
    int port;
};
ipPort_s ipPort;

typedef ipPort_s* ipPort_s_ptr;
ipPort_s_ptr ipPortList;

void parser()
{
    ipPortList =  new ipPort_s[100]; 
    std::string line;
    std::ifstream file("output.txt");
    if (!file)
    {
        cout << "Error: Cannot open file" << endl;
    }
    while(std::getline(file, line)) 
    {
        std::string token;
        std::istringstream tokens(line);
        int count = 1;
        while(std::getline(tokens, token, ',')) 
        {
            if (count == 1) 
            {
                ipPort.ipAddress = token;
                count++;
            }
            else if (count == 2)
            {
                ipPort.port = atoi(token.c_str());
            }
        }
        ipPortList[length] = ipPort;
        length++;
    }
}
int main(int argc, char *argv[])
{
    // Rakesh - uncomment it later
    char cmd[50] = "/bin/sh ./dht.sh ";
    strcat(cmd, argv[1]);
    strcat(cmd, " > input.txt");
    system(cmd);
    system("/bin/sh parser.sh");
    parser();
    string hash = argv[1];
    mysqlpp::Connection conn(false);
    mysqlpp::Query query = conn.query(); // get an object from Querry class
    if (conn.connect("", HOST, USERNAME, PASSWORD)) 
    {
        if(!conn.select_db(DB))
        {
            conn.create_db(DB); // if no database, then create it first.
            conn.select_db(DB); // select database;
        }
        
        mysqlpp::Query ifTableExist = conn.query("describe infohash_ipaddr");
        
        if (!ifTableExist.execute())
        {
            query << "CREATE TABLE infohash_ipaddr (ipaddr VARCHAR(36) not null , port INT not null , infohash VARCHAR(36) not null, PRIMARY KEY (ipaddr, infohash), UNIQUE (ipaddr, infohash))";
            if(query.execute()) // execute it!
            {
                for (int i = 0; i < length; i++)
                {
                    query.reset();
                    query << "INSERT INTO infohash_ipaddr(ipaddr, port, infohash) VALUES (\"" << ipPortList[i].ipAddress << "\", \"" << ipPortList[i].port << "\" , \"" << hash << "\")";
                    query.execute();
                }
            }
            else
            {
                cout <<"Error : Table creation Failed"<<endl;
            }
        }
        else
        {
            mysqlpp::Connection conn(false);
            mysqlpp::Query query = conn.query(); // get an object from Querry class
            if (conn.connect("", HOST, USERNAME, PASSWORD)) 
            {
                if(!conn.select_db(DB))
                {
                    conn.create_db(DB); // if no database, then create it first.
                    conn.select_db(DB); // select database;
                }
            }
            //Table is already present
            for (int i = 0; i < length; i++)
            {
                query.reset();
                query << "INSERT INTO infohash_ipaddr(ipaddr, port, infohash) VALUES (\""<<ipPortList[i].ipAddress<<"\",\""<<ipPortList[i].port<<"\",\""<<hash<<"\")";
                if (!query.execute())
                {
                    cout << "Failed"<<endl;
                }
            }
        }
    }
    else
    {
        cout<<"Error : Not able to Establish connection to database"<<endl;
    }
}

