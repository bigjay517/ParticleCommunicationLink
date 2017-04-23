#include <Base64.h>

#define MESSAGE_TX_WAIT_MS  1000
#define MESSAGE_TX_MAX_LENGTH 255

#define DEVICE_ID "bigjay517/proton/"
#define LENGTH_ID DEVICE_ID"length"
#define MESSAGE_ID DEVICE_ID"message"

char messageBuffer[1000];
char rxMessageBuffer[1000];
int rxExpectedLength;
int rxRecieved;
int rxMessageCount;
int messageWaitingTime;
int lastRxTime;

void setup() {
   Serial1.begin(115200);
   lastRxTime = messageWaitingTime = millis();
   rxExpectedLength = 0;
   for(int i=0;i<1000;i++) {
      messageBuffer[i] = 'p';
   }
   rxRecieved = 0;
   Particle.subscribe("bigjay517/core/length", setRecieveLength, MY_DEVICES);
   Particle.subscribe("bigjay517/core/message", bufferRecieveMessage, MY_DEVICES);
}

void loop() {
   if(rxRecieved>0)
   {
      if(rxRecieved != rxExpectedLength)
      {
         if(millis()-lastRxTime>=5000)
         {
            //RX TIMEOUT!!
            rxRecieved = 0;
         }
      }
      else
      {
         int decodedLen = base64_dec_len(rxMessageBuffer, rxRecieved);
         char decoded[decodedLen];
         //Serial1.print("LEN: ");
         //Serial1.println(decodedLen);

         base64_decode(decoded, rxMessageBuffer, rxRecieved);

         for(int i=0;i<decodedLen;i++)
         {
            //Serial1.print(i);
            //Serial1.print(": ");
            Serial1.print(decoded[i]);
         }

         lastRxTime = millis();
         rxRecieved = 0;
      }
   }

}

void transmitMessage(const char * buffer, const int length) {
   char txData[MESSAGE_TX_MAX_LENGTH];
   int txMessageCount = 0;
   int txTransferPending = length;
   int txLengthToSend = 0;
   int i;
   for(i=0;i<MESSAGE_TX_MAX_LENGTH;i++) {
      txData[i] = '\0';
   }
   sprintf(txData, "%d", length);
   Particle.publish(LENGTH_ID, txData, 60, PRIVATE);
   while(txTransferPending>0) {
      if(millis()-messageWaitingTime>=MESSAGE_TX_WAIT_MS) {
         if(txTransferPending>=MESSAGE_TX_MAX_LENGTH) {
            txLengthToSend = MESSAGE_TX_MAX_LENGTH;
         }
         else {
            txLengthToSend = txTransferPending;
         }
         for(i=0;i<txLengthToSend;i++) {
            txData[i]=buffer[txMessageCount*MESSAGE_TX_MAX_LENGTH+i];
         }
         txMessageCount++;
         //cout << txData << endl << "LoM: " << txLengthToSend << endl;
         Particle.publish(MESSAGE_ID, txData, 60, PRIVATE);
         messageWaitingTime=millis();
         txTransferPending-=txLengthToSend;
         for(i=0;i<MESSAGE_TX_MAX_LENGTH;i++) {
            txData[i] = '\0';
         }
      }
   }
   //Particle.publish("EOM_FROM_PROTON", length);
}

void setRecieveLength(const char *event, const char *data) {
   //char txData[10];
   rxExpectedLength = atoi(data);
   //sprintf(txData, "%d", rxExpectedLength);
   //Particle.publish("RX_LENGTH", txData);
}

void bufferRecieveMessage(const char *event, const char *data) {
   char * s = &rxMessageBuffer[rxRecieved];
   const char * t = data;
   /*
      if(rxExpectedLength-rxRecieved>MESSAGE_TX_MAX_LENGTH) {
      for(i=0;i<MESSAGE_TX_MAX_LENGTH;i++) {
      rxMessageBuffer[rxRecieved+i] = data[i];
      }
      }
      else if (rxExpectedLength-rxRecieved>0) {
      for(i=0;i<rxExpectedLength;i++) {
      rxMessageBuffer[rxRecieved+i] = data[i];
      }
      transmitMessage(rxMessageBuffer, rxExpectedLength);
      }*/
   while (*s++ = *t++)
   {
      rxRecieved++;
   }
   lastRxTime = millis();
   //transmitMessage(rxMessageBuffer, i);
   //Serial1.print("RX: ");
   //Serial1.println(rxMessageBuffer);
}
