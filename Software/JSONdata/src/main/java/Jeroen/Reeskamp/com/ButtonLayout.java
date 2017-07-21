package Jeroen.Reeskamp.com;

import javax.swing.*;

/**
 * Created by Jeroen Reeskamp on 10-4-2017.
 */
public class ButtonLayout extends JFrame {
    private JPanel mainPannel;
    private JLabel labelX;
    private JLabel labelY;
    private JLabel labelZ;
    private DrawPanel drawPanel;


    public ButtonLayout() {
        super("ButtonLayout");
        setContentPane(mainPannel);
        pack();
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        setVisible(true);
    }

    public void setLabelX(String text) {
        labelX.setText("X = " + text);
    }

    public void setLabelY(String text) {
        labelY.setText("Y = " + text);
    }

    public void setLabelZ(String text) {
        labelZ.setText("Z = " +  text);
    }

    public void updateDrawPanel(int[] coors) {
        drawPanel.update(coors);
    }

}
