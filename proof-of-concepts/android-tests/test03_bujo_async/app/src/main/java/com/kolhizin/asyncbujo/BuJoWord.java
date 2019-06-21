package com.kolhizin.asyncbujo;

import android.os.Parcel;
import android.os.Parcelable;

public class BuJoWord implements Parcelable {
    float [] xCoords;
    float [] yCoords;
    float negOffset, posOffset;

    public int describeContents(){
        return 0;
    }
    public void writeToParcel(Parcel out, int flags){
        out.writeFloat(negOffset);
        out.writeFloat(posOffset);
        out.writeFloatArray(xCoords);
        out.writeFloatArray(yCoords);
    }
    public BuJoWord(){}
    public BuJoWord(Parcel in){
        negOffset = in.readFloat();
        posOffset = in.readFloat();
        in.readFloatArray(xCoords);
        in.readFloatArray(yCoords);
    }
    public static final Creator<BuJoWord> CREATOR = new Creator<BuJoWord>() {
        @Override
        public BuJoWord createFromParcel(Parcel in) {
            return new BuJoWord(in);
        }
        @Override
        public BuJoWord[] newArray(int size) {
            return new BuJoWord[size];
        }
    };
}