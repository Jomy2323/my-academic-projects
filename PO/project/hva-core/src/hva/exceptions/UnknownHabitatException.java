package hva.exceptions;

import java.io.Serial;

public class UnknownHabitatException extends SearchException {

	@Serial
	private static final long serialVersionUID = 202404101622L;

	/** @param key */
	public UnknownHabitatException(String key) {
		super(key);
	}
}