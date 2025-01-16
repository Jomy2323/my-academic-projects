package hva.exceptions;

import java.io.Serial;
import hva.exceptions.SearchException;

public class UnknownVeterinarianException extends SearchException {

	@Serial
	private static final long serialVersionUID = 202404101626L;

	/** @param key */
	public UnknownVeterinarianException(String key) {
		super(key);
	}
}