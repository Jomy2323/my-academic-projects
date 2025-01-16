package hva.app.main;

import hva.HotelManager;
import hva.exceptions.MissingFileAssociationException;
import hva.app.main.Message;
import java.io.FileNotFoundException;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import java.io.IOException;

class DoSaveFile extends Command<HotelManager> {
    DoSaveFile(HotelManager receiver) {
        super(Label.SAVE_FILE, receiver, r -> r.getHotel() != null);
    }

    @Override
    protected final void execute() {
    	try {
            _receiver.save();
        } catch (MissingFileAssociationException e) {
            try {
                _receiver.saveAs(Form.requestString(
                    Prompt.newSaveAs()));
            } catch (MissingFileAssociationException e1) {
                e1.printStackTrace();
            } catch (IOException e1) {
                e1.printStackTrace();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
