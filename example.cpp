#include <GeoIP.h>
#include <GeoIPCity.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
#include <pthread.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <vector>
#include <mysql++.h>
#define DB "peerselector" //database name
#define HOST "localhost" // so, where's your mysql server?
#define USERNAME "root" // a user granted access to the above database?
#define PASSWORD "CareNet2011" // enter the password for the above user. If there's no password, leave it as it is...

//longitude/latitude constants
const double NAUTICALMILEPERLATITUDE = 60.00721;
const double NAUTICALMILEPERLONGITUDE = 60.10793;

//rad = math.pi / 180.0
const double MILESPERNAUTICALMILE = 1.15078;
const double KMPERNAUTICALMILE = 1.852;

struct ipAddress_s
{ 
    char* ipAddress;
    double score;
};

struct rtt_s
{ 
    char* ipAddress;
    double rtt;
};

struct thread_argument_s{
    char *criteria; 
    char *hash;
};

struct temp_data{
    string *urgentlist;
    int length;
};

typedef ipAddress_s* ipAddress_s_ptr;
typedef rtt_s* rtt_s_ptr;
typedef thread_argument_s* thread_argument_ptr;

void *getIpAddressForUrgentViews(void *arg)
{
    struct thread_argument_s *my_data;
    my_data = (struct thread_argument_s *) arg;
    struct temp_data *t = new temp_data;
    t->urgentlist = new string[50];
    char *criteria; 
    char *hash; 
    criteria = my_data->criteria;
    hash = my_data->hash;

    mysqlpp::Connection conn(false);
    mysqlpp::Query query = conn.query(); // get an object from Querry class
    if (conn.connect("", HOST, USERNAME, PASSWORD)) 
    {
        if(!conn.select_db(DB))
        {
            conn.create_db(DB); // if no database, then create it first.
            conn.select_db(DB); // select database;
        }

        if (!strcmp(criteria,"0"))
        {
            query << "SELECT ipaddr FROM asn_table WHERE infohash = \"" << hash << "\" ORDER BY score";
            if (mysqlpp::StoreQueryResult res = query.store()) 
            {
			    mysqlpp::StoreQueryResult::const_iterator it;
			    int i = 0;
			    for (it = res.begin(); it != res.end(); ++it,++i)
                {
				    mysqlpp::Row row = *it;
				    t->urgentlist[i] = const_cast<char *>(row["ipaddr"].c_str());
			    }

                t->length = i;
            }
            else
                cerr << "Failed to get item list: " << query.error() << endl;
		}
        else if (!strcmp(criteria,"1"))
        {
            query << "SELECT ipaddr FROM rtt_table WHERE infohash = \"" << hash << "\" ORDER BY rtt";
            if (mysqlpp::StoreQueryResult res = query.store()) 
            {
			    mysqlpp::StoreQueryResult::const_iterator it;
			    int i = 0;
			    for (it = res.begin(); it != res.end(); ++it,++i)
                {
				    mysqlpp::Row row = *it;
				    t->urgentlist[i] = const_cast<char *>(row["ipaddr"].c_str());
			    }

                t->length = i;
            }
            else
                cerr << "Failed to get item list: " << query.error() << endl;
		}
		else 
        {
			cerr << "Failed to get item list: " << query.error() << endl;
		}
    	return (void *)t;
    }
}

class Geo
{
public:
Geo(char *myIp)
{
    m_myIp = new char[30]; 
    m_myIp = myIp;
    cout<<"My Ip address : "<< m_myIp << endl;
}
~Geo()
{ 
    if (m_myIp)
    {
         //delete[] m_myIp;
          //m_myIp = 0;
    }

    if (m_gi)
    {
        GeoIP_delete(m_gi); 
    }

    if (m_ASgi)
    {
        GeoIP_delete(m_ASgi); 
    }
} // Need to delete the pointer

double calculateDistance(double peerLatitude, double peerLongitude, double myLatitude, double myLongitude)
{
    double yDistance = (myLatitude - peerLatitude) * NAUTICALMILEPERLATITUDE;
    double xDistance = (cos(peerLatitude * (M_PI/180.0)) + cos(myLatitude * (M_PI/180.0))) * (myLongitude - peerLongitude) * (NAUTICALMILEPERLONGITUDE / 2);
    double distance = sqrt(pow(yDistance,2) + pow(xDistance,2));
    return int((distance * KMPERNAUTICALMILE) + .5);
}

void getRecords(char* ipAddress, string &continent, string & country, string &city)
{
     try
     {
	    GeoIPRecord  *gir = GeoIP_record_by_addr(m_gi, ipAddress);
	    string temp;
        if (gir)
        {
            if (gir->continent_code)
                continent = gir->continent_code;
            if (gir->country_name)
                country = gir->country_name;
            if (gir->city)
                city = gir->city;
            /*cout << "========================================================================="<<endl;
            cout << "Ip Address ->" <<ipAddress<<endl;
            cout << "Continent ->"<<continent<<endl;
            cout << "Country ->" <<country<<endl;
            cout << "City ->" <<city<<endl;
            cout << "========================================================================="<<endl;*/
            GeoIPRecord_delete(gir); 
        }

    }
    catch(...)
   {
   }
}

bool isInSameCity(char* peerIp)
{
    string continent;
    string country;
    string city;
    string myContinent;
    string myCountry;
    string myCity;
    getRecords(peerIp, continent, country, city);
    getRecords(m_myIp, myContinent, myCountry, myCity);
    if (city == myCity)
    {
        cout <<"Peers are from same city"<<endl;
	    return true;
    }
    return false;
}

bool isInSameContinent(char* peerIp)
{
    string continent;
    string country;
    string city;
    string myContinent;
    string myCountry;
    string myCity;
    getRecords(peerIp, continent, country, city);
    getRecords(m_myIp, myContinent, myCountry, myCity);
    if (continent == myContinent)
    {
        cout <<"Peers are from same continent"<<endl;
	    return true;
    }
    return false;
}

bool isInSameCountry(char* peerIp)
{
    string continent;
    string country;
    string city;
    string myContinent;
    string myCountry;
    string myCity;
    getRecords(peerIp, continent, country, city);
    getRecords(m_myIp, myContinent, myCountry, myCity);
    if (country == myCountry)
    {
        cout <<"Peers are from same country"<<endl;
	    return true;
    }
    return false;

}
char * getProvider(char* ipAddress)
{
    try
    {
        // Wrong - need to buy ISP database. GeoIPISP.dat. is not free. sp m_ASgi should be replaced once we buy the database
	    char * org = GeoIP_org_by_name(m_ASgi, ipAddress);
        if (org)
        {
             return org;
        }
        else
        { 
            return NULL;
        }
    }
    catch(...)
    {
    }
}

bool isInSameProvider(char* peerIp)
{
    if (!strcmp(getProvider(peerIp),getProvider(m_myIp)))
    {
        cout <<"Peers are from same Provider"<<endl;
	    return true;
    }
    return false;
}

char * getASN(char* ipAddress)
{
    try
    {
        char *org = GeoIP_org_by_addr(m_ASgi, ipAddress);
        if (org)
        {
            return org;
        }
        else
        { 
            //char returnString[40];
            char *returnString = "ASN is not known";
            //strcpy (returnString, "ASN is not known");

            return returnString;
        }
    }
    catch(...)
    {
    }
}

bool isInSameASN(char* peerIp)
{
 
 char* a = getASN(peerIp);
 char* b = getASN(m_myIp);
 if (!strcmp(a,b))
    //if (!strcmp(getASN(peerIp),getASN(m_myIp)))
    {
        cout <<"Peers are from same ASN"<<endl;
	    return true;
    }
    return false;
}

int calculateScore(char* peerIp)
{
    double peerLatitude = 0;
    double peerLongitude = 0;
    double myLatitude = 0;
    double myLongitude = 0;
    int extraKm = 0;
    double totalScore = -1;
    cout<<"================================================================================"<<endl;
    cout<<"Peer IP address : "<< peerIp << endl;
    getCoordinates(peerIp, peerLatitude, peerLongitude);
    getCoordinates(m_myIp, myLatitude, myLongitude);
            
    if (peerLatitude<=0 || peerLongitude <=0)
    {
         cout<< "Invalid Peer IP address -> " <<peerIp<< endl;
         return totalScore;
    }
    
    if (myLatitude<=0 || myLongitude <=0)
    {
         cout<< "Invalid Own IP address -> " <<m_myIp<< endl;
         return totalScore;
    }
    double distance = calculateDistance(peerLatitude, peerLongitude, myLatitude, myLongitude);
    cout << "Geographic distance between two peer:->" << distance << endl; 

    if (isInSameASN(peerIp))
    {
        extraKm = 1;
    }
    /*else if (isInSameProvider(peerIp))
    {
        extraKm = 10;    
    }*/
    else if (isInSameCity(peerIp))
    {
        extraKm = 100;
    }
    else if (isInSameCountry(peerIp))
    {
        extraKm = 200;
    }
    else if (isInSameContinent(peerIp))
    {
        extraKm = 300;
    }
    else
    {
        extraKm = 500;
    }       
    totalScore = distance + extraKm;
    return totalScore;
}

void initialize()
{
    m_gi = GeoIP_open("/usr/share/GeoIP/GeoIPCity.dat", GEOIP_MEMORY_CACHE);
    if (m_gi == NULL) {
        fprintf(stderr, "Error opening database\n");
	exit(1);
    }
    
    m_ASgi = GeoIP_open("/usr/share/GeoIP/GeoIPASNum.dat", GEOIP_STANDARD);
    if (m_ASgi == NULL) {
        fprintf(stderr, "Error opening database\n");
	exit(1);
    }
}

void getCoordinates(char *ip, double &latitude, double & longitude)
{
    try
    {
        GeoIPRecord    *gir = GeoIP_record_by_addr(m_gi, ip);
        if (gir)
        {
            cout <<"Latitude : " << gir->latitude << endl;
            cout <<"Longitude :" << gir->longitude << endl;
            latitude = gir->latitude;
            longitude = gir->longitude;
        }
    }
    catch (...)
    {
    }    
}

void sortIP(ipAddress_s ipAddressList[], int length)
{
     ipAddress_s temp;

     for(int i = 1; i < length - 1; i++)
     {
          for (int j = i + 1; j < length; j++)
          {
               if (ipAddressList[i].score > ipAddressList[j].score)  //comparing score
               {
                     temp = ipAddressList[i];    //swapping entire struct
                     ipAddressList[i] = ipAddressList[j];
                     ipAddressList[j] = temp;
               }
          }
     }
     return;
}


void sortRTT(rtt_s rttList[], int length)
{
     rtt_s temp;

     for(int i = 1; i < length - 1; i++)
     {
          for (int j = i + 1; j < length; j++)
          {
               if (rttList[i].rtt > rttList[j].rtt)  //comparing rtt
               {
                     temp = rttList[i];    //swapping entire struct
                     rttList[i] = rttList[j];
                     rttList[j] = temp;
               }
          }
     }
     return;
}

double calculateRtt(char* ipAddress)
{

    FILE *fp;
    char rttBuffer[20];
    double SampleRTT = 0;
    double EstimatedRTT = 0;
    double Deviation = 0;
    double RTO = -1;
    double x = .1;
    for (int i = 0; i < 10; i++)
    {
        char cmd[50] = "/bin/sh rtt.sh ";
        strcat(cmd, ipAddress);
         
        fp = popen(cmd, "r");

        if (fp == NULL) 
        {
            cout << "Failed to run command"<< endl;
            exit(1);
        }

        /* Read the output a line at a time - output it. */
        while (fgets(rttBuffer, sizeof(rttBuffer)-1, fp) != NULL) 
        {
            cout << "RTT ->  " << rttBuffer;
        }
        SampleRTT = atof(rttBuffer);
        if (RTO == -1) 
        {
            EstimatedRTT = SampleRTT;
            Deviation = SampleRTT / 2.0;
            RTO = EstimatedRTT + 4 * Deviation;
        } 
        else 
        {
            // since the IP address is unreachable 2nd time, so there is no point trying again 
            if (EstimatedRTT == 0)
                break;
            Deviation = (1 - x) * Deviation + x * abs(EstimatedRTT - SampleRTT);
		    EstimatedRTT = (1 - x) * EstimatedRTT + x * SampleRTT;
            // This time out is for future use, right now I am relying on ping protocol for timeout
            RTO = EstimatedRTT + 4 * Deviation;
        }

        pclose(fp);
    }
    return EstimatedRTT;
}

void getIpAddress(string list[], char *hash, int *length)
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

        query << "SELECT ipaddr FROM infohash_ipaddr WHERE infohash = \"" << hash << "\"";
        if (mysqlpp::StoreQueryResult res = query.store()) {
			cout << "We have:" << endl;
			mysqlpp::StoreQueryResult::const_iterator it;
			int i = 0;
			for (it = res.begin(); it != res.end(); ++it,++i) {
				mysqlpp::Row row = *it;
				cout << '\t' << row["ipaddr"] << endl;
                list[i] = const_cast<char *>(row["ipaddr"].c_str());
                //strcpy(list[i],const_cast<char *>(row["ipaddr"].c_str()));
			}
            *length = i;
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
		}
    }
}

void storePeerInAsnTable(ipAddress_s ipAddressList[], int length, char *hash)
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
        
        mysqlpp::Query ifTableExist = conn.query("describe asn_table");
        
        if (!ifTableExist.execute())
        {
            query << "CREATE TABLE asn_table (ipaddr VARCHAR(36) not null , score DOUBLE not null , infohash VARCHAR(36) not null, PRIMARY KEY (ipaddr, infohash), UNIQUE (ipaddr, infohash))";
            if(query.execute()) // execute it!
            {
                for (int i = 0; i < length; i++)
                {
                    query.reset();
                    query << "INSERT INTO asn_table(ipaddr, score, infohash) VALUES (\"" << ipAddressList[i].ipAddress << "\", \"" << ipAddressList[i].score << "\" , \"" << hash << "\")";
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
                //Table is already present
                for (int i = 0; i < length; i++)
                {
                    query.reset();
                    query << "INSERT INTO asn_table(ipaddr, score, infohash) VALUES (\""<<ipAddressList[i].ipAddress<<"\",\""<<ipAddressList[i].score<<"\",\""<<hash<<"\")";
                    if (!query.execute())
                    {
                        query.reset();
                        query << "UPDATE asn_table SET score = (\""<<ipAddressList[i].score<<"\") where ipaddr = (\""<<ipAddressList[i].ipAddress<<"\") AND infohash = (\""<<hash<<"\")";
                        query.execute();
                    }
                }
            }
        }
    }
    else
    {
        cout<<"Error : Not able to Establish connection to database"<<endl;
    }
}

void storePeerInRttTable(rtt_s ipAddressList[], int length, char *hash)
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
        
        mysqlpp::Query ifTableExist = conn.query("describe rtt_table");
        
        if (!ifTableExist.execute())
        {
            query << "CREATE TABLE rtt_table (ipaddr VARCHAR(36) not null , rtt DOUBLE not null , infohash VARCHAR(36) not null, PRIMARY KEY (ipaddr, infohash), UNIQUE (ipaddr, infohash))";
            if(query.execute()) // execute it!
            {
                for (int i = 0; i < length; i++)
                {
                    query.reset();
                    query << "INSERT INTO rtt_table(ipaddr, rtt, infohash) VALUES (\"" << ipAddressList[i].ipAddress << "\", \"" << ipAddressList[i].rtt << "\" , \"" << hash << "\")";
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
                //Table is already present
                for (int i = 0; i < length; i++)
                {
                    query.reset();
                    query << "INSERT INTO rtt_table(ipaddr, rtt, infohash) VALUES (\""<<ipAddressList[i].ipAddress<<"\",\""<<ipAddressList[i].rtt<<"\",\""<<hash<<"\")";
                    if (!query.execute())
                    {
                        query.reset();
                        query << "SELECT rtt FROM rtt_table WHERE ipaddr = \"" << ipAddressList[i].ipAddress << "\" AND infohash = \"" << hash << "\"";
                        if (mysqlpp::StoreQueryResult res = query.store()) 
                        {
                            if (abs(ipAddressList[i].rtt - res[0][0]) > 1)
                            {
                                query.reset();
                                query << "UPDATE rtt_table SET rtt = (\""<<ipAddressList[i].rtt<<"\") where ipaddr = (\""<<ipAddressList[i].ipAddress<<"\") AND infohash = (\""<<hash<<"\")";
                            }
			                
		                }
		                else 
		                {
			                cerr << "Failed to get item list: " << query.error() << endl;
		                }
                    }
                }
            }
        }
    }
    else
    {
        cout<<"Error : Not able to Establish connection to database"<<endl;
    }
}

private:
char *m_myIp;
GeoIP *m_gi;
GeoIP *m_ASgi;
};

int main (int argc, char *argv[]) {

    // call geolitecityupdate.sh to get MaxMind database
    system("/bin/sh geolitecityupdate.sh");
    system("/bin/sh geoIPASNum.sh");

    if(argc <= 2)
    {
        cout<< "You must provide score/rtt(0/1) flag and at least two IP Addresses\n"<< endl;
        exit(1);
    }
    string list[50];
    int listLength;
    string *urgentList;
    int urgentListLength;
    char *myIpAddress = argv[3];
    Geo objGeo = Geo(myIpAddress);

    pthread_t pth;	// this is our thread identifier
    struct thread_argument_s thread_arg;
    thread_arg.criteria = argv[1];
    thread_arg.hash = argv[2];
    
	/* Create worker thread */
	pthread_create(&pth,NULL,getIpAddressForUrgentViews,(void *) &thread_arg);

    void *temp;
    struct temp_data *tempReturn;

   	/* wait for our thread to finish before continuing */
    pthread_join(pth,(void **) &temp);
    tempReturn = (struct temp_data *) temp;
    urgentList = tempReturn->urgentlist;
    urgentListLength = tempReturn->length;

    for (int i = 0; i < urgentListLength; i++)
    {
        cout<<"Urgent List :"<< urgentList[i] << endl;
    }
    objGeo.getIpAddress(list, argv[2], &listLength);
    objGeo.initialize();

    // Calculate best peer based on Geographic location
    if (!strcmp(argv[1],"0"))
    {
        ipAddress_s_ptr ipAddressList;
        ipAddressList =  new ipAddress_s[listLength];
        ipAddressList[0].ipAddress = argv[3];
        ipAddressList[0].score = 0;
        int length = listLength + 1;
        for (int i = 0, j = 1; i < listLength; i ++)
        {
            char *peerIpAddress = const_cast<char *>(list[i].c_str());
            double peerScore = objGeo.calculateScore(peerIpAddress);
            if (peerScore > 0)
            {
                ipAddressList[j].score = peerScore;
                ipAddressList[j].ipAddress = const_cast<char *>(list[i].c_str());
                j++;
            }
            else
            {
                length = length - 1;
            }
        }

        cout<<"================================================================================"<<endl;
        cout<<"Best peer based on score"<<endl;
        objGeo.sortIP(ipAddressList, length);
        for (int i = 1; i < length; i++)
        {
            cout<<"Peer :"<< ipAddressList[i].ipAddress <<" -> Score :"<< ipAddressList[i].score << endl;
        }
        objGeo.storePeerInAsnTable(ipAddressList, length, argv [2]);
    }
    // calculate best peer based on RTT
    else if (!strcmp(argv[1],"1"))
    {
        rtt_s_ptr rttList;
        rttList =  new rtt_s[listLength];
        rttList[0].ipAddress = argv[3];
        rttList[0].rtt = 0;
        int rttlength = listLength + 1;
        for (int i = 0, k = 1; i < listLength; i ++)
        {
            char *peerIpAddress = const_cast<char *>(list[i].c_str());
            double rtt = objGeo.calculateRtt(peerIpAddress);
            if (rtt > 0)
            {
                rttList[k].rtt = rtt;
                rttList[k].ipAddress = const_cast<char *>(list[i].c_str());
                k++;
            }
            else
            {
                rttlength = rttlength - 1;
            }
        }

        cout<<"================================================================================"<<endl;
        cout<<"Best peer based on rtt"<<endl;
        objGeo.sortRTT(rttList, rttlength);
        for (int i = 1; i < rttlength; i ++)
        {
            cout<<"Peer :"<< rttList[i].ipAddress <<" -> RTT :"<< rttList[i].rtt << endl;
        }
        objGeo.storePeerInRttTable(rttList, rttlength, argv [2]);
    }
}

