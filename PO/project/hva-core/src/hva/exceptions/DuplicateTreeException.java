package hva.exceptions;

import java.io.Serial;

public class DuplicateTreeException extends Exception {

	@Serial
	private static final long serialVersionUID = 202404101617L;

	/** The tree's key. */
	private final String _key;

	/** @param key */
	public DuplicateTreeException(String key) {
		_key = key;
	}

  	/** @return the key */
  	public String getKey() {
		return _key;
  	}
}