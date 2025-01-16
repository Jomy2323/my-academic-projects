package hva.exceptions;

import java.io.Serial;

public class DuplicateAnimalException extends Exception {

	@Serial
	private static final long serialVersionUID = 202404101615L;

	/** The animal's key. */
	private final String _key;

	/** @param key */
	public DuplicateAnimalException(String key) {
    	_key = key;
	}

  	/** @return the key */
  	public String getKey() {
    	return _key;
  	}

}