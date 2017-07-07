package com.sitebkns.display;

//2017.07.07 BTC Culled down from https://github.com/Pixplicity/android-bluetooth-demo

//Refs
// https://hackaday.io/project/10314-raspberry-pi-3-as-an-eddystone-url-beacon
// https://developer.mbed.org/teams/Bluetooth-Low-Energy/code/BLE_EddystoneBeacon_Service/file/dfb7fb5a971b/Eddystone.h
// https://github.com/TakahikoKawasaki/nv-bluetooth

// https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile  (a bit complicated to digest)

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothAdapter.LeScanCallback;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.neovisionaries.bluetooth.ble.advertising.ADPayloadParser;
import com.neovisionaries.bluetooth.ble.advertising.ADStructure;
import com.sitebkns.display.R;

import java.util.List;

/**
 * Main screen of the application. This is where all the magic happens!
 * Usually, to keep your code clean you'd want to move all the logic concerning bluetooth to a
 * separate controller, a {@link Service service}, a utility class or a combination of those.
 */
public class MainActivity extends AppCompatActivity implements LeScanCallback {

    /**
     * Request code used when starting the bluetooth settings Activity
     */
    private static final int REQUEST_ENABLE_BT = RESULT_FIRST_USER;

    private static final String TAG = MainActivity.class.getSimpleName();

    // UI elements
    private Button mBtScan;
    private View mScanResults;
    private TextView mTvScanResults;
    private ProgressBar mProgress;

    // Bluetooth objects
    private BluetoothAdapter mAdapter;

    // App logic
    private boolean mIsScanning = false;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Initialize views
        mBtScan = (Button) findViewById(R.id.bt_scan);
        mProgress = (ProgressBar) findViewById(R.id.busy);
        mScanResults = findViewById(R.id.scan_results);
        mTvScanResults = (TextView) findViewById(R.id.tv_scan_results);

        // Button listeners
        mBtScan.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(@NonNull View view) {
                startScan();
            }
        });

        // Initialize the adapter
        BluetoothManager btManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mAdapter = btManager.getAdapter();
        if (!isBluetoothEnabled()) {
            // If bluetooth is not enabled, we ask the user to do so
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
            // If the permission android.permission.BLUETOOTH_ADMIN is included in the manifest,
            // then we could also enable bluetooth without requiring a user action by doing:
            // mAdapter.enable();
            // ...But that is not nice towards the user.
        }

        //startScan();  // 2017.07.07.BTC speed up code-test cycle.  Delete for normal app operation.
    }


    /**
     * Called for every device that is found during the scan
     *
     * @param device     The device that is found
     * @param rssi       The received signal strength indication
     * @param scanRecord Extra data concerning the scanned device
     */
    @Override
    public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord) {
        //Ref- https://developer.android.com/reference/android/bluetooth/BluetoothDevice.html
        mScanResults.setVisibility(View.VISIBLE);
        // Filter Raspberry Pi Foundation OUI=B8, to reduce noise from other devices in vicinity. ""=disabled.
        String macxFilter = "B8";
        if(( macxFilter.length() == 0) || device.getAddress().startsWith(macxFilter)) {
            fullDebugPrintScanRecord( device, rssi, scanRecord );
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        // Enable the scan button if bluetooth is enabled and if
        // it is not already scanning
        mBtScan.setEnabled(isBluetoothEnabled() && !mIsScanning);
    }


    @Override
    protected void onPause() {
        super.onPause();
        if (mIsScanning && mAdapter != null) {
            // Stop scanning! We're done!
            mAdapter.stopLeScan(this);
            scanStopped();
        }
     }


    /**
     * Checks if bluetooth is enabled in the system settings
     *
     * @return {@code true} if bluetooth is available and enabled, {@code false} otherwise
     */
    public boolean isBluetoothEnabled() {
        return mAdapter != null && mAdapter.isEnabled();
    }


    /**
     * Start scanning for nearby devices. If a UUID is entered in the EditText, it will scan
     * for that specific service. If a MAC address or nothing is entered it will scan for all
     * devices.
     *
     * @see #onLeScan(BluetoothDevice, int, byte[])
     */
    private void startScan() {
        mIsScanning = true;
        mProgress.setVisibility(View.VISIBLE);
        mBtScan.setEnabled(false);
         mAdapter.startLeScan(this);
    }


    /**
     * Enables the UI elements to start a new scan.
     */
    private void scanStopped() {
        mIsScanning = false;
        mProgress.setVisibility(View.INVISIBLE);
        mBtScan.setEnabled(true);
    }


    public void fullDebugPrintScanRecord( BluetoothDevice device, int rssi, byte[] scanRecord ) {
        mTvScanResults.append(getString(R.string.found_device, device.getAddress()));
        mTvScanResults.append(bytesToHex(scanRecord));
        List<ADStructure> structures = ADPayloadParser.getInstance().parse(scanRecord);
        for (ADStructure structure : structures) {
            String[] fields = structure.toString().split(",");
            for ( int j = 0; j < fields.length; j++ ) {
                mTvScanResults.append("  " + fields[j] + "\n");
            }
        }
        mTvScanResults.append("\n");
    }


    private final static char[] hexArray = "0123456789ABCDEF-".toCharArray();
    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 3];
        for ( int j = 0; j < bytes.length; j++ ) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 3] = hexArray[v >>> 4];
            hexChars[j * 3 + 1] = hexArray[v & 0x0F];
            hexChars[j * 3 + 2] = hexArray[16];
        }
        return new String(hexChars);
    }

    //======================
    // ACTIVITY LINKS
    //======================

    /** Called when the user taps the Send button */
    public static final String EXTRA_MESSAGE = "com.sitebkns.display.MESSAGE";
    public void invokeSecondActivity(View view) {
        Intent intent = new Intent(this, SecondActivity.class);
        intent.putExtra(EXTRA_MESSAGE, "Hey there! Go Team BKNS...");
        startActivity(intent);
    }

    CustomCanvasView customCanvasView;
    public void invokeMapActivity(View view) {
        Intent intent = new Intent(this, MapActivity.class);
        startActivity(intent);
    }

}
