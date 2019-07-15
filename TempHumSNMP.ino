/************************************************************
 * Monitor temperature and humidity over SNMP               *                                                        *
 * *********************************************************/
#include <Streaming.h>         // Include the Streaming library
#include <Ethernet.h>          // Include the Ethernet library
#include <SPI.h>
#include <MemoryFree.h>
#include <Agentuino.h> 
#include <Flash.h>

#include "DHT.h"
#define DHTPIN 2     // groove pin for tempsensor
#define DHTTYPE DHT11   // DHT 11 type tempsensor


DHT dht(DHTPIN, DHTTYPE);


static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  //Mac Address for Arduino Ethernet Shield
static byte ip[] = { 192, 168, 178, 6 };                      //IP Address for Arduino Ethernet Shield
//static byte gateway[] = { 192, 168, 178, 1 };
//static byte subnet[] = { 255, 255, 255, 0 };

const int kPinTemp = A0;                                     //Lm35 Analog Channel Input
int temp=0;                                                 //Centigrade Temperature variable
int hum=0;                                                 //Farenheight Temperatur variable

// RFC1213-MIB OIDs
// .iso (.1)
// .iso.org (.1.3)
// .iso.org.dod (.1.3.6)
// .iso.org.dod.internet (.1.3.6.1)
// .iso.org.dod.internet.mgmt (.1.3.6.1.2)
// .iso.org.dod.internet.mgmt.mib-2 (.1.3.6.1.2.1)
// .iso.org.dod.internet.mgmt.mib-2.system (.1.3.6.1.2.1.1)
// .iso.org.dod.internet.mgmt.mib-2.system.sysDescr (.1.3.6.1.2.1.1.1)
// .iso.org.dod.internet.mgmt.mib-2.system.sysContact (.1.3.6.1.2.1.1.4)
// .iso.org.dod.internet.mgmt.mib-2.system.sysName (.1.3.6.1.2.1.1.5)
// .iso.org.dod.internet.mgmt.mib-2.system.sysLocation (.1.3.6.1.2.1.1.6)
// .iso.org.dod.internet.mgmt.mib-2.system.sysServices (.1.3.6.1.2.1.1.7)
const char sysDescr[] PROGMEM      = "1.3.6.1.2.1.1.1.0";  // System Description
const char sysContact[] PROGMEM    = "1.3.6.1.2.1.1.4.0";  // System Contact
const char sysName[] PROGMEM       = "1.3.6.1.2.1.1.5.0";  // System Name
const char sysLocation[] PROGMEM   = "1.3.6.1.2.1.1.6.0";  // System Location
const char sysServices[] PROGMEM   = "1.3.6.1.2.1.1.7.0";  // System Services


//My Custom OID's
const char temperatureC[]       PROGMEM     = "1.3.6.1.3.2019.5.1.0";     //Temperature in Celsius
const char humidity[]           PROGMEM     = "1.3.6.1.3.2019.5.1.1";     //Temperature is Fahrenheit


// RFC1213 local values
static char locDescr[]              = "Read temperature/moisture dht11";                 // read-only (static)
static char locContact[50]          = "Vossius serverruimte";             // should be stored/read from EEPROM - read/write (not done for simplicity)
static char locName[20]             = "F.P. Vonck";                              // should be stored/read from EEPROM - read/write (not done for simplicity)
static char locLocation[20]         = "Amsterdam";                          // should be stored/read from EEPROM - read/write (not done for simplicity)
static int32_t locServices          = 2;                                             // read-only (static)

uint32_t prevMillis = millis();
char oid[SNMP_MAX_OID_LEN];
SNMP_API_STAT_CODES api_status;
SNMP_ERR_CODES status;

void pduReceived()
{
  SNMP_PDU pdu;
  api_status = Agentuino.requestPdu(&pdu);
  //
  if ((pdu.type == SNMP_PDU_GET || pdu.type == SNMP_PDU_GET_NEXT || pdu.type == SNMP_PDU_SET)
    && pdu.error == SNMP_ERR_NO_ERROR && api_status == SNMP_API_STAT_SUCCESS ) {
    //
    pdu.OID.toString(oid);
    
    if ( strcmp_P(oid, sysDescr ) == 0 ) {
      // handle sysDescr (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } else {
        // response packet from get-request - locDescr
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locDescr);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } 
    } else if ( strcmp_P(oid, sysName ) == 0 ) {
      // handle sysName (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locName, strlen(locName)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locName
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locName);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else if ( strcmp_P(oid, sysContact ) == 0 ) {
      // handle sysContact (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locContact, strlen(locContact)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locContact
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locContact);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else if ( strcmp_P(oid, sysLocation ) == 0 ) {
      // handle sysLocation (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locLocation, strlen(locLocation)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locLocation
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locLocation);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    } else if ( strcmp_P(oid, sysServices) == 0 ) {
      // handle sysServices (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } else {
        // response packet from get-request - locServices
        status = pdu.VALUE.encode(SNMP_SYNTAX_INT, locServices);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
    }
    else if ( strcmp_P(oid, temperatureC ) == 0 ) /////////////////////////////////////////////////////////////Temperature in Centigrade
    {
      // handle sysName (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) 
      {      
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } 
      else 
      {
        status = pdu.VALUE.encode(SNMP_SYNTAX_INT, temp);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
    } 
        else if ( strcmp_P(oid, humidity ) == 0 ) ///////////////////////////////////////////////////////////Temperature in Farenheight
    {
      // handle sysName (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) 
      {      
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } 
      else 
      {
        status = pdu.VALUE.encode(SNMP_SYNTAX_INT, hum);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
    }   
    else {
      // oid does not exist
      // response packet - object not found
      pdu.type = SNMP_PDU_RESPONSE;
      pdu.error = SNMP_ERR_NO_SUCH_NAME;
    }
    Agentuino.responsePdu(&pdu);
  }

  Agentuino.freePdu(&pdu);
 
}

void setup()
{

  // start dht tempsensor
  dht.begin();

  Ethernet.begin(mac, ip);                     //Initialize Ethernet Shield
  api_status = Agentuino.begin();              //Begin Snmp agent on Ethernet shield

  if ( api_status == SNMP_API_STAT_SUCCESS ) {
    Agentuino.onPduReceive(pduReceived);
    delay(10);
    return;
  }
  delay(10);
}

void loop()
{

 // measure temp en humidity
 float h = dht.readHumidity();
 float t = dht.readTemperature();
  
 Agentuino.listen();                                      //Listen/Handle for incoming SNMP requests
                
 
 temp=(int)t;                                       //Update Centigrade temperature variable
 hum=(int)h;                                       //Update Farenheight temperature variable
}
