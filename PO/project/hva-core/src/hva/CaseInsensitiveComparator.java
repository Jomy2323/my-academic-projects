package hva;

import java.util.Comparator;
import java.io.Serializable;
import java.io.Serial;

//FIXME perguntar ao professor se isto está ok
public class CaseInsensitiveComparator implements
Comparator<String>, Serializable {
    @Serial
    private static final long serialVersionUID = 202410101319L;

    @Override
    public int compare(String s1, String s2) {
       return s1.compareToIgnoreCase(s2);
    }
}