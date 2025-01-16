package hva.exceptions;

import java.io.Serial;

public class DuplicateHabitatException extends Exception {

	@Serial
	private static final long serialVersionUID = 202404101616L;

	/** The habitat's key. */
	private final String _key;

	/** @param key */
	public DuplicateHabitatException(String key) {
		_key = key;
	}

  	/** @return the key */
  	public String getKey() {
		return _key;
  	}
}