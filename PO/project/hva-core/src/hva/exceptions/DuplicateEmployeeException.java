package hva.exceptions;

import java.io.Serial;

public class DuplicateEmployeeException extends Exception {

	@Serial
	private static final long serialVersionUID = 202404101614L;

	/** The employee's key. */
	private final String _key;

	/** @param key */
	public DuplicateEmployeeException(String key) {
		_key = key;
	}

  	/** @return the key */
  	public String getKey() {
		return _key;
  	}

}