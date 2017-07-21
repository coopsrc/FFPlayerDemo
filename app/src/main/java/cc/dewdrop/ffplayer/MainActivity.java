package cc.dewdrop.ffplayer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import cc.dewdrop.ffplayer.utils.FFUtils;

public class MainActivity extends AppCompatActivity {

    private TextView mTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTextView = (TextView) findViewById(R.id.sample_text);
    }

    public void onButtonClick(View view) {
        int id = view.getId();

        switch (id) {
            case R.id.button_protocol:
                setInfoText(FFUtils.urlProtocolInfo());
                break;
            case R.id.button_codec:
                setInfoText(FFUtils.avCodecInfo());
                break;
            case R.id.button_filter:
                setInfoText(FFUtils.avFilterInfo());
                break;
            case R.id.button_format:
                setInfoText(FFUtils.avFormatInfo());
                break;
        }
    }

    private void setInfoText(String content) {
        if (mTextView != null) {
            mTextView.setText(content);
        }
    }
}
