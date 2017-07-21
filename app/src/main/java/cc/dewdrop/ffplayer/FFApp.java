package cc.dewdrop.ffplayer;

import android.app.Application;

/**
 * Created by Zhang Tingkuo.
 * Date: 2017-07-21
 * Time: 15:28
 */

public class FFApp extends Application {

    static {
        System.loadLibrary("native-lib");
    }

}
