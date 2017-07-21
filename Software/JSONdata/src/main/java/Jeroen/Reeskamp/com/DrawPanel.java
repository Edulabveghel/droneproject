package Jeroen.Reeskamp.com;

import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.io.File;
import java.io.IOException;
import java.util.Vector;

/**
 * Created by Jeroen Reeskamp on 10-4-2017.
 */
public class DrawPanel extends JPanel {
    public static final int X = 987;
    public static final int Y = 791;

    private Vector<int[]> coordinates;
    Image img;

    public DrawPanel() {
        coordinates = new Vector<>();
        try {
            img = ImageIO.read(new File("src/main/res/images/map.png"));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void paintComponent(Graphics g) {
        g.drawImage(img,0,0,null);
        g.setColor(Color.RED);
        if(coordinates.size() > 1) {
            for (int i = 0; i < coordinates.size() - 1; i++) {
                g.drawLine(coordinates.elementAt(i)[0], coordinates.elementAt(i)[1],
                        coordinates.elementAt(i + 1)[0], coordinates.elementAt(i + 1)[1]);
            }
            g.fillOval(coordinates.elementAt(coordinates.size() - 1)[0] - 20,coordinates.elementAt(coordinates.size() - 1)[1] - 20,40,40);
        }
    }

    @Override
    public Dimension getPreferredSize() {
        return new Dimension(X,Y);
    }

    public void update(int[] coors) {
        coordinates.add(coors);
        repaint();
    }

}
