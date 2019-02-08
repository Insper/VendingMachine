package hm10.insper.com.vendingmachine;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.*;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.*;
import org.w3c.dom.Text;

import java.util.UUID;

public class HM10TerminalActivity extends Activity {

    public static final String EXTRAS_DEVICE_NAME = "DEVICE_NAME";
    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";
    private static final String TAG = "HM10TerminalActivity";

    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothDevice mBluetoothDevice;
    private BluetoothGatt mBluetoothGatt;
    private boolean connected = false;

    private UUID hm10UartUUID = UUID.fromString("0000ffe0-0000-1000-8000-00805f9b34fb");
    private UUID hm10UartWriteUUID = UUID.fromString("0000ffe1-0000-1000-8000-00805f9b34fb");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        final BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        Bundle bundle = getIntent().getExtras();
        mBluetoothDevice = mBluetoothAdapter.getRemoteDevice(bundle.getString(EXTRAS_DEVICE_ADDRESS));
        super.onCreate(savedInstanceState);

        //setContentView(R.layout.activity_hm10terminal);
        setContentView(R.layout.activity_vending);
        final Button button = (Button) findViewById(R.id.button);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (connected) {
                    BluetoothGattCharacteristic ch = mBluetoothGatt.getService(hm10UartUUID).getCharacteristic(hm10UartWriteUUID);
                    final EditText editText = (EditText) findViewById(R.id.editText2);

                    ch.setValue(editText.getText().toString());
                    Log.i(TAG, "Enviando input.");
                    mBluetoothGatt.writeCharacteristic(ch);
                    editText.setText("");
                }
            }
        });

        final Button button_debug = (Button) findViewById(R.id.debug_button);
        button_debug.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                final LinearLayout debug_view = (LinearLayout) findViewById(R.id.debug_layout);
                debug_view.setVisibility(View.VISIBLE);

            }
        });

        final Button button_1 = (Button) findViewById(R.id.cor1_comprar);
        button_1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (connected) {
                    BluetoothGattCharacteristic ch = mBluetoothGatt.getService(hm10UartUUID).getCharacteristic(hm10UartWriteUUID);
                    ch.setValue("C;0;1\n");
                    Log.i(TAG, "Enviando compra cor 1.");
                    mBluetoothGatt.writeCharacteristic(ch);
                }

            }
        });

        final Button button_2 = (Button) findViewById(R.id.cor2_comprar);
        button_2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (connected) {
                    BluetoothGattCharacteristic ch = mBluetoothGatt.getService(hm10UartUUID).getCharacteristic(hm10UartWriteUUID);
                    ch.setValue("C;1;1\n");
                    Log.i(TAG, "Enviando compra cor 2.");
                    mBluetoothGatt.writeCharacteristic(ch);
                }

            }
        });
    }

    @Override
    protected void onResume() {
        mBluetoothGatt = mBluetoothDevice.connectGatt(this, false, mBluetoothGattCallback);
        super.onResume();
    }

    @Override
    protected void onPause() {
        mBluetoothGatt.disconnect();
        super.onPause();
    }

    private final BluetoothGattCallback mBluetoothGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "Connected to GATT server.");
                mBluetoothGatt.discoverServices();
                connected = true;
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, "Disconnected to GATT server.");
                connected = false;
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            for (final BluetoothGattService service : gatt.getServices()) {
                Log.i(TAG, String.format("Servico encontrado: %s", service.getUuid()));

                for (final BluetoothGattCharacteristic ch : service.getCharacteristics()) {
                    Log.i(TAG, String.format("Característica encontrado: %s", ch.getUuid()));

                    if (ch.getUuid().equals(hm10UartWriteUUID)) {
                        BluetoothGattDescriptor descriptor =
                                ch.getDescriptor(convertFromInteger(0x2902));
                        if (descriptor != null) {
                            gatt.setCharacteristicNotification(ch, true);
                            Log.i(TAG, String.format("Mudando descritor para notificar", ch.getUuid()));
                            descriptor.setValue(
                                    BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                            gatt.writeDescriptor(descriptor);
                        }
                    }
                }
            }

            super.onServicesDiscovered(gatt, status);
        }

        public UUID convertFromInteger(int i) {
            final long MSB = 0x0000000000001000L;
            final long LSB = 0x800000805f9b34fbL;
            long value = i & 0xFFFFFFFF;
            return new UUID(MSB | (value << 32), LSB);
        }


        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, final BluetoothGattCharacteristic characteristic) {
            final String receivedData = characteristic.getStringValue(0).trim();
            Log.i(TAG, String.format("Recebi : %s", receivedData));
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if(receivedData.startsWith("S")) {
                        String filamentoData[] = receivedData.split(";");
                        if(filamentoData[0].equals("S0")) {
                            final TextView cor1 = (TextView) findViewById(R.id.cor1_texto);
                            cor1.setText("Cor: " + filamentoData[1]);

                            final TextView qnt1 = (TextView) findViewById(R.id.cor1_qnt);
                            qnt1.setText("Quantidade: " + filamentoData[2]);
                        } else if(filamentoData[0].equals("S1")) {
                            final TextView cor2 = (TextView) findViewById(R.id.cor2_texto);
                            cor2.setText("Cor: " + filamentoData[1]);

                            final TextView qnt2 = (TextView) findViewById(R.id.cor2_qnt);
                            qnt2.setText("Quantidade: " + filamentoData[2]);
                        }
                    }
                    if(receivedData.startsWith("ROK")) {
                        Toast.makeText(HM10TerminalActivity.this, "Compra realizada com sucesso!", Toast.LENGTH_SHORT).show();
                    }

                    EditText terminal = (EditText) findViewById(R.id.editText1);
                    terminal.setText(terminal.getText() + "\n" + receivedData);
                   terminal.setSelection(terminal.getText().length());
                }
            });


            super.onCharacteristicChanged(gatt, characteristic);
        }


    };
}
