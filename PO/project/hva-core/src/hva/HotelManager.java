package hva;

import java.io.*;
import hva.exceptions.*;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
//FIXME import other Java classes
//FIXME import other project classes

/**
 * Class that represents the hotel application.
 */
public class HotelManager {

    /** The hotel manager. */
    private String _filename = "";

    /** This is the current hotel. */
    private Hotel _hotel = new Hotel();

    // FIXME maybe add more fields if needed

    /**
     * Saves the serialized application's state into the file
     * associated to the current network.
     *
     * @throws FileNotFoundException if for some reason the file
     * cannot be created or opened.
     * @throws MissingFileAssociationException if the current network
     * does not have a file.
     * @throws IOException if there is some error while serializing
     * the state of the network to disk.
     */
    public void save() throws FileNotFoundException,
     MissingFileAssociationException, IOException {
        if (_filename == null || _filename.equals(""))
            throw new MissingFileAssociationException();
        try (ObjectOutputStream oos = 
        new ObjectOutputStream(new BufferedOutputStream(new
         FileOutputStream(_filename)))) {
            oos.writeObject(_hotel);
            _hotel.setChanged(false);
        }
    }

    /**
     * Saves the serialized application's state into the file
     * associated to the current network.
     *
     * @throws FileNotFoundException if for some reason the file
     * cannot be created or opened.
     * @throws MissingFileAssociationException if the current network
     * does not have a file.
     * @throws IOException if there is some error while serializing
     * the state of the network to disk.
     */
    public void saveAs(String filename) throws FileNotFoundException,
     MissingFileAssociationException, IOException {
        _filename = filename;
        save();
    }

    /**
     * @param filename name of the file containing the serialized
     * application's state
     *        to load.
     * @throws UnavailableFileException if the specified file does not
     * exist or there is an error while processing this file.
     */
    public void load(String filename) throws UnavailableFileException,
     IOException, ClassNotFoundException {
        _filename = filename;
        try (ObjectInputStream ois =
         new ObjectInputStream(new BufferedInputStream(new
          FileInputStream(filename)))) {
            _hotel = (Hotel) ois.readObject();
            _hotel.setChanged(false);
        }
    }

    /**
     * Read text input file.
     *
     * @param filename name of the text input file
     * @throws ImportFileException
     */
    public void importFile(String filename) throws
     ImportFileException {
        _hotel.importFile(filename);
    }

    public Hotel getHotel(){
        return _hotel;
    }

    public boolean changed() {
        return _hotel.hasChanged();
    }

    public void reset() {
        _hotel = new Hotel();
        _filename = null;
    }

    public String getFilename() {
        return _filename;
    }

    /**
     * @param filename
     */
    public void setFilename(String filename) {
        _filename = filename;
    }
}
