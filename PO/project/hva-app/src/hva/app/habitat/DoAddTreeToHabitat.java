package hva.app.habitat;

import hva.Hotel;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.exceptions.DuplicateTreeException;
import hva.exceptions.UnknownHabitatException;
import hva.app.exceptions.DuplicateTreeKeyException;
import hva.app.exceptions.UnknownHabitatKeyException;
import hva.Tree;

class DoAddTreeToHabitat extends Command<Hotel> {

    DoAddTreeToHabitat(Hotel receiver) {
        super(Label.ADD_TREE_TO_HABITAT, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try{
            Form request = new Form();
            request.addStringField("habitatKey", Prompt.habitatKey());
            request.addStringField("treeKey", Prompt.treeKey());
            request.addStringField("treeName", Prompt.treeName());
            request.addStringField("treeAge", Prompt.treeAge());
            request.addStringField("treeDifficulty",
             Prompt.treeDifficulty());
            request.parse();

            String type;
            do{
                type = Form.requestString(Prompt.treeType());
            } while (!type.equals("CADUCA") &&
             !type.equals("PERENE"));

            String treeKey = request.stringField("treeKey");
            String treeName = request.stringField("treeName");
            String treeAge = request.stringField("treeAge");
            String treeDifficulty = request.stringField
             ("treeDifficulty");
            String habitatKey = request.stringField("habitatKey");
            _receiver.registerTree(new String[]{null, treeKey,
                 treeName, treeAge, treeDifficulty, type, habitatKey
                });

            
            _display.popup(_receiver.addTreeToHabitat(habitatKey,
             treeKey));
            
        } catch (UnknownHabitatException e){
            throw new UnknownHabitatKeyException(e.getKey());
        } catch (DuplicateTreeException e){
            throw new DuplicateTreeKeyException(e.getKey());
        }
    }

}
