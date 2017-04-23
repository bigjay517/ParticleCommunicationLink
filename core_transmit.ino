#include <Base64.h>

#define MESSAGE_TX_WAIT_MS  1000
#define MESSAGE_TX_MAX_LENGTH 255

#define DEVICE_ID "bigjay517/core/"
#define LENGTH_ID DEVICE_ID"length"
#define MESSAGE_ID DEVICE_ID"message"

char messageBuffer[1000];
char rxMessageBuffer[1000];
int rxExpectedLength;
int rxRecieved;
int rxMessageCount;
int messageWaitingTime;
int lastRxTime;
int serialLastRx;
int msgBufIndex;

void setup() {
   Serial1.begin(115200);
   msgBufIndex = 0;
   lastRxTime = serialLastRx = messageWaitingTime = millis();
   rxExpectedLength = 0;
   for(int i=0;i<1000;i++) {
      messageBuffer[i] = '\0';
   }
   rxRecieved = 0;
   Particle.subscribe("bigjay517/proton/length", setRecieveLength, MY_DEVICES);
   Particle.subscribe("bigjay517/proton/message", bufferRecieveMessage, MY_DEVICES);
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

         base64_decode(decoded, rxMessageBuffer, rxRecieved);

         for(int i=0;i<decodedLen;i++)
         {
            Serial1.print(decoded[i]);
         }

         lastRxTime = millis();
         rxRecieved = 0;
      }
   }

   if (Serial1.available()) {
      char inByte = Serial1.read();
      /*Serial1.print(inByte);*/
      messageBuffer[msgBufIndex++] = (char)inByte;
      serialLastRx = millis();
   }
   if (msgBufIndex>0) {
      if(millis()-serialLastRx>=500)
      {
         int encodedLen = base64_enc_len(msgBufIndex);
         char encoded[encodedLen];
         base64_encode(encoded, messageBuffer, msgBufIndex); 
         transmitMessage(encoded, encodedLen);
         msgBufIndex=0;
         for(int i=0;i<1000;i++) {
            messageBuffer[i] = '\0';
         }
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
         Particle.publish(MESSAGE_ID, txData, 60, PRIVATE);
         messageWaitingTime=millis();
         txTransferPending-=txLengthToSend;
         for(i=0;i<MESSAGE_TX_MAX_LENGTH;i++) {
            txData[i] = '\0';
         }
      }
   }
}

void setRecieveLength(const char *event, const char *data) {
   rxExpectedLength = atoi(data);
}

void bufferRecieveMessage(const char *event, const char *data) {
   char * s = &rxMessageBuffer[rxRecieved];
   const char * t = data;
   while (*s++ = *t++)
   {
      rxRecieved++;
   }
   lastRxTime = millis();
}
