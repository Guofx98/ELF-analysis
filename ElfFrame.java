
import javax.swing.*;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableModel;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.*;

public class ElfFrame extends JFrame implements ActionListener , MouseListener {
    private static final long serialVersionUID=-9077023825514749548L;
    JPanel up,down;
    JFileChooser jfc;
    JList list;
    DefaultListModel dlm;
    JTextField text[];
    JTextField tf;
    JTable jTable;
    DefaultTableModel dtm;
    public ElfFrame(){
        super("ELF文件解析器");
        this.getContentPane().setBackground(Color.WHITE);
        this.setSize(850,600);
        this.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        this.setLayout(new BorderLayout());

        up=new JPanel(new BorderLayout());
        JPanel jp1=new JPanel(new FlowLayout());
        jp1.setBackground(Color.WHITE);
        jp1.add(new JLabel("Magic   "));
        tf=new JTextField(47);
        tf.setEditable(false);
        tf.setBackground(Color.WHITE);
        jp1.add(tf);
        up.add(jp1,BorderLayout.NORTH);
        up.setBackground(Color.WHITE);
        JPanel jp2=new JPanel(new GridLayout(4,8,12,7));
        jp2.setBackground(Color.WHITE);
        text=new JTextField[15] ;
        for(int i=0;i<15;i++){
            text[i]=new JTextField(10);
            text[i].setEditable(false);
            text[i].setBackground(Color.WHITE);
        }
        jp2.add(new JLabel("  解析文件名"));
        jp2.add(text[0]);
        jp2.add(new JLabel("  ELF对象"));
        jp2.add(text[1]);
        jp2.add(new JLabel("  文件类型"));
        jp2.add(text[2]);
        jp2.add(new JLabel("  计算机结构"));
        jp2.add(text[3]);
        jp2.add(new JLabel("  版本"));
        jp2.add(text[4]);
        jp2.add(new JLabel("  入口点地址"));
        jp2.add(text[5]);
        jp2.add(new JLabel("  程序头地址"));
        jp2.add(text[6]);
        jp2.add(new JLabel("  节头表地址"));
        jp2.add(text[7]);
        jp2.add(new JLabel("  处理器特殊标识"));
        jp2.add(text[8]);
        jp2.add(new JLabel("  ELF文件头长度"));
        jp2.add(text[9]);
        jp2.add(new JLabel("  程序头长度"));
        jp2.add(text[10]);
        jp2.add(new JLabel("  程序头数"));
        jp2.add(text[11]);
        jp2.add(new JLabel("  节头长度"));
        jp2.add(text[12]);
        jp2.add(new JLabel("  节头数"));
        jp2.add(text[13]);
        jp2.add(new JLabel("  字符串表索引长度"));
        jp2.add(text[14]);
        jp2.add(new JLabel());
        JButton btn=new JButton("打  开");
        btn.addActionListener(this);
        jp2.add(btn);
        up.add(jp2,BorderLayout.SOUTH);
        JSplitPane sp1=new JSplitPane(JSplitPane.VERTICAL_SPLIT);

        sp1.add(up,JSplitPane.TOP);
        this.add(sp1,BorderLayout.CENTER);

        jfc=new JFileChooser();
        jfc.setCurrentDirectory(new File("d://share"));   //txt文件所在的目录

        down=new JPanel(new BorderLayout());
        dlm = new DefaultListModel();
        list=new JList();
        list.setModel(dlm);
        list.addMouseListener(this);
        JScrollPane jsp = new JScrollPane(list);
        JSplitPane sp2=new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);

        sp2.add(jsp,JSplitPane.LEFT);
        down.add(sp2,BorderLayout.CENTER);
        String [] head={"[index]","名称","类型","地址"};
        dtm=new DefaultTableModel(head,0);
        jTable=new JTable(dtm);
        jTable.setBackground(Color.WHITE);
        JScrollPane jsp1 = new JScrollPane(jTable);
        jsp1.getViewport().setBackground(Color.WHITE);
        sp2.add(jsp1,JSplitPane.RIGHT);
        sp1.add(down,JSplitPane.BOTTOM);

        this.setVisible(true);
        sp1.setDividerLocation(0.29);
        sp2.setDividerLocation(0.22);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        if(e.getActionCommand().equals("打  开")){
            int state = jfc.showOpenDialog(null);
            if (state == 1) {
            } else {
                File file = jfc.getSelectedFile();
                try {
                    boolean b=display(file);
                    if(b==false){
                        JOptionPane.showMessageDialog(null,"该文件格式错误！","错误",1);
                    }
                    else{
                        dlm.addElement(file);
                        list.setModel(dlm);
                    }
                } catch (IOException ioException) {
                    ioException.printStackTrace();
                }
            }

        }
    }
    public boolean display(File file) throws IOException {
        String filename=file.getName();
        filename=filename.substring(0,filename.length()-4);
        FileInputStream fis=new FileInputStream(file);
        InputStreamReader fin=new InputStreamReader(fis);
        BufferedReader bfr=new BufferedReader(fin);
        String str=bfr.readLine();
        String [] ss=str.split(" ");
        if(ss[0].equals("Magic")){
            this.text[0].setText(filename);
            if(ss[6].equals("1")){
                this.text[1].setText("ELF32");
            }
            else
                this.text[1].setText("ELF64");
            String magic="";
            for(int i=2;i<ss.length;i++){
                magic+=ss[i]+" ";
            }
            this.tf.setText(magic);
            int i=2;
            while((str=bfr.readLine())!=null&&i<15){
                String [] temp=str.split(" ");
                this.text[i++].setText("0x"+temp[2]);
            }
            i=0;
            dtm.setRowCount(0);
            while((str=bfr.readLine())!=null){
                String [] index=str.split("\t");
                dtm.insertRow(i++,index);
                jTable.setModel(dtm);

            }
            return true;
        }
        else
            return false;
    }
    public static void main(String args[]){
        ElfFrame elf=new ElfFrame();
    }

    @Override
    public void mouseClicked(MouseEvent e) {
        if(list.getSelectedIndex() != -1) {
            if (e.getClickCount() == 2){
                File file= (File) list.getSelectedValue();
                try {
                    display(file);
                } catch (IOException ioException) {
                    ioException.printStackTrace();
                }
            }
        }
    }

    @Override
    public void mousePressed(MouseEvent e) {

    }

    @Override
    public void mouseReleased(MouseEvent e) {

    }

    @Override
    public void mouseEntered(MouseEvent e) {

    }

    @Override
    public void mouseExited(MouseEvent e) {

    }
}
