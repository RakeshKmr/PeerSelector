#include <GeoIP.h>
#include <GeoIPCity.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <vector>

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

typedef ipAddress_s* ipAddress_s_ptr;

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
        if (gir)
        {
            continent = gir->continent_code;
            country = gir->country_name;
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
            
    if (peerLatitude<=0 && peerLongitude <=0)
    {
         cout<< "Invalid IP address -> " <<peerIp<< endl;
         return totalScore;
    }
    
    if (myLatitude<=0 && myLongitude <=0)
    {
         cout<< "Invalid IP address -> " <<m_myIp<< endl;
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
    cout<< "You must provide at least Two IP Addresses\n"<< endl;
        exit(1);
    }
    ipAddress_s_ptr ipAddressList;
    ipAddressList =  new ipAddress_s[argc];
        
    ipAddressList[1].ipAddress = argv[1];
    char *myIpAddress = argv[1];
    ipAddressList[1].score = 0;
    int length = argc;

    Geo objGeo = Geo(myIpAddress);
    objGeo.initialize();

    for (int i = 2,  j = 2; i < argc; i ++)
    {
        char *peerIpAddress = argv[i];
        double peerScore = objGeo.calculateScore(peerIpAddress);
        if (peerScore >=0)
        {
            ipAddressList[j].score = peerScore;
            ipAddressList[j].ipAddress = argv[i];
            j++;
         }
         else
         {
            length = length - 1;
         }
    }

    cout<<"================================================================================"<<endl;
    objGeo.sortIP(ipAddressList, length);
    for (int i = 1; i < length; i ++)
    {
        cout<<"Peer :"<< ipAddressList[i].ipAddress <<" -> Score :"<< ipAddressList[i].score << endl;
    }
}

