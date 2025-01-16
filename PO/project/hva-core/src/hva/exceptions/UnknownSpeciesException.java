package hva.exceptions;

import java.io.Serial;

public class UnknownSpeciesException extends Exception {

	@Serial
	private static final long serialVersionUID = 202404101623L;

	/** The species' key. */
	private final String _key;

	/** @param key */
	public UnknownSpeciesException(String key) {
		_key = key;
	}

  	/** @return the key */
  	public String getKey() {
		return _key;
  	}

}