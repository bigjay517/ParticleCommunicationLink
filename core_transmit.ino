#include <Base64.h>

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
   Serial1.begin(115200);
   msgBufIndex = 0;
   testPublishTime = messageWaitingTime = millis();
   for(int i=0;i<1000;i++) {
      messageBuffer[i] = '\0';
   }
   delay(1000);

#if (0)
   Serial1.println("Comm enabled..");

   //BASE 64 SAMPLE CODE
   char input[] = "Hello world";
   int inputLen = sizeof(input);

   int encodedLen = base64_enc_len(inputLen);
   char encoded[encodedLen];

   Serial1.print(input); Serial1.print(" = ");

   // note input is consumed in this step: it will be empty afterwards
   base64_encode(encoded, input, inputLen); 

   Serial1.println(encoded);
   /*transmitMessage(encoded, encodedLen);*/
   // decoding
   char input2[] = "Zm9vYmFy";
   int input2Len = sizeof(input2);

   int decodedLen = base64_dec_len(encoded, encodedLen);
   char decoded[decodedLen];
   
   Serial1.print(encoded); Serial1.print(" = "); 
   base64_decode(decoded, encoded, encodedLen);

   Serial1.println(decoded);
   //BASE 64 SAMPLE CODE
#endif
}

void loop() {
   if (Serial1.available()) {
      char inByte = Serial1.read();
      /*Serial1.print(inByte);*/
      messageBuffer[msgBufIndex++] = (char)inByte;
      testPublishTime = millis();
   }
   if (msgBufIndex>0) {
      if(millis()-testPublishTime>=500)
      {
         int encodedLen = base64_enc_len(msgBufIndex);
         char encoded[encodedLen];
         base64_encode(encoded, messageBuffer, msgBufIndex); 
         /*
         Serial1.print("Input: ");
         for(int i=0;i<msgBufIndex;i++)
         {
            Serial1.print(messageBuffer[i], HEX);
         }
         Serial1.println("");
         Serial1.print("Encod: ");
         Serial1.println(encoded);
         Serial1.print("EnLen: ");
         Serial1.println(encodedLen);
         */
         transmitMessage(encoded, encodedLen);
         msgBufIndex=0;
         for(int i=0;i<1000;i++) {
            messageBuffer[i] = '\0';
         }
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
   for(i=0;i<MESSAGE_TX_MAX_LENGTH;i++) {
      txData[i] = '\0';
   }
   sprintf(txData, "%d", length);
   /*
   Serial1.print("Buffe: ");
   Serial1.println(buffer);
   Serial1.print("Lengt: ");
   Serial1.println(length);
   */
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
