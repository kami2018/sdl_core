package com.ford.syncV4.android.adapters;

import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import com.ford.syncV4.android.R;
import com.ford.syncV4.proxy.RPCMessage;
import com.ford.syncV4.proxy.constants.Names;
import com.ford.syncV4.proxy.rpc.enums.Result;
import com.ford.syncV4.util.logger.Logger;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;

public class MessageAdapter extends ArrayAdapter<Object> {

    @SuppressWarnings("unused")
    private static final String LOG_TAG = MessageAdapter.class.getSimpleName();

    public MessageAdapter(Context context, int textViewResourceId, ArrayList<Object> items) {
        super(context, textViewResourceId, items);
    }

    /**
     * Adds the specified message to the items list and notifies of the change.
     */
    public void addMessage(Object m) {
        add(m);
    }

    static class ViewHolder {
        TextView lblTop;
        TextView lblBottom;
    }

    public View getView(int position, View convertView, ViewGroup parent) {

        ViewHolder holder;
        TextView lblTop;
        TextView lblBottom;

        ViewGroup rowView = (ViewGroup) convertView;
        if (rowView == null) {
            LayoutInflater layoutInflater =
                    (LayoutInflater) getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            rowView = (ViewGroup) layoutInflater.inflate(R.layout.row, null);

            lblTop = (TextView) rowView.findViewById(R.id.toptext);
            lblBottom = (TextView) rowView.findViewById(R.id.bottomtext);

            holder = new ViewHolder();
            holder.lblTop = lblTop;
            holder.lblBottom = lblBottom;
            rowView.setTag(holder);
        } else {
            holder = (ViewHolder) rowView.getTag();
            lblTop = holder.lblTop;
            lblBottom = holder.lblBottom;

            lblBottom.setVisibility(View.VISIBLE);
            lblBottom.setText(null);
            lblTop.setTextColor(getContext().getResources().getColor(R.color.app_regular_text_color));
            lblTop.setText(null);
        }

        Object rpcObj = getItem(position);
        if (rpcObj != null) {
            if (rpcObj instanceof String) {
                lblTop.setText((String) rpcObj);
                lblBottom.setVisibility(View.GONE);
            } else if (rpcObj instanceof RPCMessage) {
                RPCMessage func = (RPCMessage) rpcObj;
                if (func.getMessageType().equals(Names.request)) {
                    lblTop.setTextColor(Color.CYAN);
                } else if (func.getMessageType().equals(Names.notification)) {
                    lblTop.setTextColor(Color.YELLOW);
                } else if (func.getMessageType().equals(Names.response)) {
                    lblTop.setTextColor(Color.argb(255, 32, 161, 32));
                }

                lblTop.setText(func.getFunctionName() + " (" + func.getMessageType() + ")");

                try {
                    Method getSuccessMethod = rpcObj.getClass().getMethod("getSuccess");
                    boolean isSuccess = (Boolean) getSuccessMethod.invoke(func);
                    if (isSuccess) {
                        lblTop.setTextColor(Color.GREEN);
                    } else {
                        lblTop.setTextColor(Color.RED);
                    }
                    Method getInfoMethod = rpcObj.getClass().getMethod("getInfo");
                    Method getResultCodeMethod = rpcObj.getClass().getMethod("getResultCode");

                    String info = (String) getInfoMethod.invoke(rpcObj);
                    Result result = (Result) getResultCodeMethod.invoke(rpcObj);

                    lblBottom.setText(result + (info != null ? ": " + info : ""));

                } catch (NoSuchMethodException e) {
                    lblBottom.setVisibility(View.GONE);
                } catch (SecurityException e) {
                    Logger.e("MessageAdapter", e.toString());
                } catch (IllegalArgumentException e) {
                    Logger.e("MessageAdapter", e.toString());
                } catch (IllegalAccessException e) {
                    Logger.e("MessageAdapter", e.toString());
                } catch (InvocationTargetException e) {
                    Logger.e("MessageAdapter", e.toString());
                }
            }
        }

        return rowView;
    }
}