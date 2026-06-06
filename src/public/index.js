let selectedCell = null;

document.addEventListener('DOMContentLoaded', () => {
    const cells = document.querySelectorAll('.cell');
    const pickerCells = document.querySelectorAll('.picker-cell');
    const checkButton = document.getElementById('check-sudoku');

    // Select a cell on click (only mutable cells)
    cells.forEach(cell => {
        cell.addEventListener('click', () => {
            if (!cell.classList.contains('mutable')) return;

            if (selectedCell) {
                selectedCell.classList.remove('selected');
            }

            selectedCell = cell;
            cell.classList.add('selected');
        });
    });

    // Update selected cell when picker is clicked
    pickerCells.forEach(pickerCell => {
        pickerCell.addEventListener('click', () => {
            if (selectedCell && selectedCell.classList.contains('mutable')) {
                selectedCell.textContent = pickerCell.textContent;
            }
        });
    });

    checkButton.addEventListener('click', async () => {
        const allCells = document.querySelectorAll('.sudoku .cell');
        let board = '';

        for (const cell of allCells) {
            const val = cell.textContent.trim();
            if (val === '') {
                // Sudoku is not finished — do nothing
                return;
            }
            board += val;
        }

        try {
            const response = await fetch(`/check?board=${board}`);
            const text = await response.text();
            console.log(text);
            const valid = text.split('valid=')[1] === '1';
            checkButton.textContent = valid ? 'Valid!' : 'Invalid!';
            checkButton.className = valid ? 'check-btn valid' : 'check-btn invalid';
            setTimeout(() => {
                checkButton.textContent = 'Check Sudoku';
                checkButton.className = 'check-btn';
            }, 1000);
        } catch (err) {
            checkButton.textContent = 'Server error!';
            console.log(err.text);
            checkButton.className = 'check-btn invalid';
            setTimeout(() => {
                checkButton.textContent = 'Check Sudoku';
                checkButton.className = 'check-btn';
            }, 1000);
        }
    });

});
