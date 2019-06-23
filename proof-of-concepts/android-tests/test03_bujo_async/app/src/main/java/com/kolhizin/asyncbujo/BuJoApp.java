package com.kolhizin.asyncbujo;

import android.app.Application;

public class BuJoApp extends Application {
    private BuJoPage curPage = null;

    public BuJoPage getPage(){return curPage;}
    public void setPage(BuJoPage page){curPage = page;}
}
