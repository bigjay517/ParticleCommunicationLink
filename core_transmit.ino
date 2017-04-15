#define MESSAGE_TX_WAIT_MS  1000
#define MESSAGE_TX_MAX_LENGTH 255

#define DEVICE_ID "bigjay517/core/"
#define LENGTH_ID DEVICE_ID"length"
#define MESSAGE_ID DEVICE_ID"message"

char messageBuffer[1000];
int messageWaitingTime;
int testPublishTime;
int msgBufIndex;

void setup() {
   Serial.begin(115200);
   msgBufIndex = 0;
   testPublishTime = messageWaitingTime = millis();
   for(int i=0;i<1000;i++) {
      messageBuffer[i] = '\0';
   }
}

void loop() {
   if (Serial.available()) {
      int inByte = Serial.read();
      messageBuffer[msgBufIndex++] = (char)inByte;
      testPublishTime = millis();
   }
   if (msgBufIndex>0) {
      if(millis()-testPublishTime>=500)
      {
         transmitMessage(messageBuffer, msgBufIndex);
         msgBufIndex=0;
      }
   }



   /* No need for test message to be transmitted */
   /*
   if(millis()-testPublishTime>=60000)
   {
      //Particle.publish("30 second event");
      transmitMessage(messageBuffer, 1000);
      testPublishTime = millis();
   }
   */

}

void transmitMessage(const char * buffer, const int length) {
   char txData[MESSAGE_TX_MAX_LENGTH];
   int txMessageCount = 0;
   int txTransferPending = length;
   int txLengthToSend = 0;
   int i;
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
