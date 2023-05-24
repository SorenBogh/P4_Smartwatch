/*
TODO: Omregn til bpm

*/
int currentVal = 0;
int previousVal = 0;
int maxVal = 0;
int minVal = 0;
int checkVal = 0;
float diff = 0;
bool once = false;
float Comp[9];
float maxval = 0;  //max værdi i forhold til diff
float minval = 0;  //min værdi i arrayet til bestemmelse af diff
int BPM = 0;
int currentBeat;
int lastBeat;


void finddiff() {
  for (int i = 0; i < 10; i++) {
    Comp[i] = analogRead(36);
    delay(75);
  }

  maxval = Comp[0];
  minval = Comp[0];

  for (int i = 0; i < (10 / sizeof(Comp[0])); i++) {
    maxval = max(Comp[i], maxval);
    minval = min(Comp[i], minval);
  }
  diff = maxval - minval;
}

void setup() {
  // put your setup code here, to run once
  pinMode(36, INPUT);
  Serial.begin(115200);
  //lastVal = analogRead(36);
}

void loop() {
  // put your main code here, to run repeatedly:
  millis();
  finddiff();
  previousVal = currentVal;
  currentVal = analogRead(36);

  if (currentVal > maxVal) {
    maxVal = currentVal;
  }

  if (currentVal < minVal) {
    minVal = currentVal;
  }

  checkVal = (maxVal - minVal);




  if (currentVal >= checkVal && once == false) {
    once = true;
    lastBeat = currentBeat;
    currentBeat = millis();
    BPM = ((currentBeat - lastBeat) * 60) / 1000;
    if ((BPM > 45) && (BPM < 110)) {
      Serial.println(BPM);
    }


  } else if (currentVal <= checkVal - 10) {
    once = true;
  }
  delay(10);
}
