package hva.exceptions;

import java.io.Serial;

public class SearchException extends Exception {

    @Serial
	private static final long serialVersionUID = 202422100912L;

    /** The object's key. */
	private final String _key;

    /** @param key */
    public SearchException(String key) {
        _key = key;
    }

    /** @return the key */
  	public String getKey() {
    	return _key;
  	}
}