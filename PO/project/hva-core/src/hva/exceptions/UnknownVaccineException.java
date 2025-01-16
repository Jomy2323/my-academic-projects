package hva.exceptions;

import java.io.Serial;

public class UnknownVaccineException extends Exception {

	@Serial
	private static final long serialVersionUID = 202404101625L;

	/** The vaccine's key. */
	private final String _key;

	/** @param key */
	public UnknownVaccineException(String key) {
		_key = key;
	}

  	/** @return the key */
  	public String getKey() {
		return _key;
  	}
}