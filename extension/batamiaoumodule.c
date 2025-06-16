#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <structmember.h>
#include <stdlib.h>
#include <time.h>

#define BATAMIAOU_N 36
#define BATAMIAOU_HALF_N 18
#define BATAMIAOU_ERROR_END -999999
#define BATAMIAOU_PLAYER_1 1
#define BATAMIAOU_PLAYER_2 2

typedef struct
{
    int deck[BATAMIAOU_N];
    int player_cards[2];
} BatamiaouGame;

static int batamiaou_shuffle(BatamiaouGame *game, int player)
{
    for (int i = 0; i < BATAMIAOU_N; i++)
    {
        if (game->deck[i] == -player)
        {
            game->deck[i] = player;
            game->player_cards[player - 1] += 1;
        }
    }
    return game->player_cards[player - 1];
}

static int batamiaou_get_card_index(BatamiaouGame *game, int player)
{
    if (game->player_cards[player - 1] <= 0)
    {
        int k = batamiaou_shuffle(game, player);
        if (k > 0)
        {
            return batamiaou_get_card_index(game, player);
        }
        return BATAMIAOU_ERROR_END;
    }

    const int j = rand() % game->player_cards[player - 1];

    int k = 0;
    for (int i = 0; i < BATAMIAOU_N; i++)
    {
        if (game->deck[i] == player)
        {
            if (k == j)
            {
                return i;
            }
            k++;
        }
    }
    return BATAMIAOU_ERROR_END;
}

static void batamiaou_get_card(BatamiaouGame *game, int player, int *index, int *value)
{
    int const card_index = batamiaou_get_card_index(game, player);
    if (card_index == BATAMIAOU_ERROR_END)
    {
        *index = BATAMIAOU_ERROR_END;
        *value = BATAMIAOU_ERROR_END;
        return;
    }
    game->player_cards[player - 1] -= 1;
    game->deck[card_index] = 0;

    *index = card_index;
    *value = (card_index % 6) + 1;
    return;
}

static void batamiaou_win(BatamiaouGame *game, int i0, int i1, int player)
{
    game->deck[i0] = -player;
    game->deck[i1] = -player;
    for (int i = 0; i < BATAMIAOU_N; i++)
    {
        if (game->deck[i] == 0)
        {
            game->deck[i] = -player;
        }
    }
}

static int batamiaou_play(BatamiaouGame *game)
{
    int i1 = 0, i2 = 0, p1 = 0, p2 = 0;
    batamiaou_get_card(game, BATAMIAOU_PLAYER_1, &i1, &p1);
    batamiaou_get_card(game, BATAMIAOU_PLAYER_2, &i2, &p2);
    if (i1 == BATAMIAOU_ERROR_END || i2 == BATAMIAOU_ERROR_END)
    {
        return BATAMIAOU_ERROR_END;
    }

    if (p1 > p2)
    {
        batamiaou_win(game, i1, i2, BATAMIAOU_PLAYER_1);
        return BATAMIAOU_PLAYER_1;
    }
    else if (p2 > p1)
    {
        batamiaou_win(game, i1, i2, BATAMIAOU_PLAYER_2);
        return BATAMIAOU_PLAYER_2;
    }
    else
    {
        batamiaou_get_card(game, BATAMIAOU_PLAYER_1, &i1, &p1);
        batamiaou_get_card(game, BATAMIAOU_PLAYER_2, &i2, &p2);
        return batamiaou_play(game);
    }
}

static void batamiaou_init(BatamiaouGame *game)
{
    int j = 0;
    int k = 0;
    for (int i = 0; i < BATAMIAOU_N; i++)
    {
        game->deck[i] = BATAMIAOU_PLAYER_1;
    }
    while (k < BATAMIAOU_HALF_N)
    {
        j = rand() % BATAMIAOU_N;
        if (game->deck[j] != BATAMIAOU_PLAYER_2)
        {
            game->deck[j] = BATAMIAOU_PLAYER_2;
            k++;
        }
    }
    game->player_cards[0] = BATAMIAOU_HALF_N;
    game->player_cards[1] = BATAMIAOU_HALF_N;
}

static int batamiaou_run(BatamiaouGame *game)
{
    int k = 0;
    int r = 0;
    while (r != BATAMIAOU_ERROR_END)
    {
        r = batamiaou_play(game);
        k++;
    }
    return k - 1;
}

// Python C extension ===================================================== //

// Python object definition
typedef struct
{
    PyObject_HEAD
        BatamiaouGame game;
} PyBatamiaouGame;

// __new__
static PyObject *
PyBatamiaouGame_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyBatamiaouGame *self;
    self = (PyBatamiaouGame *)PyType_GenericAlloc((PyTypeObject *)type, 0);
    if (self != NULL)
    {
        memset(&self->game, 0, sizeof(BatamiaouGame));
    }
    return (PyObject *)self;
}

// __init__
static int
PyBatamiaouGame_init(PyBatamiaouGame *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"seed", NULL};
    unsigned int seed = (unsigned int)time(NULL);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|I", kwlist, &seed))
        return -1;

    srand(seed);
    batamiaou_init(&self->game);
    return 0;
}

// .init() method
static PyObject *
PyBatamiaouGame_reset(PyBatamiaouGame *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"seed", NULL};
    unsigned int seed = (unsigned int)time(NULL);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|I", kwlist, &seed))
        Py_RETURN_NONE;

    srand(seed);
    batamiaou_init(&self->game);
    Py_RETURN_NONE;
}

// .run() method
static PyObject *
PyBatamiaouGame_run(PyBatamiaouGame *self, PyObject *Py_UNUSED(ignored))
{
    int rounds = batamiaou_run(&self->game);
    return PyLong_FromLong(rounds);
}

// .play() method
static PyObject *
PyBatamiaouGame_play(PyBatamiaouGame *self, PyObject *Py_UNUSED(ignored))
{
    int result = batamiaou_play(&self->game);
    if (result == BATAMIAOU_ERROR_END)
    {
        Py_RETURN_NONE;
    }
    return PyLong_FromLong(result);
}

// .deck() method
static PyObject *PyBatamiaouGame_deck(PyBatamiaouGame *self, PyObject *Py_UNUSED(ignored))
{
    return PyBytes_FromStringAndSize((const char *)(&(self->game.deck[0])), sizeof(int) * BATAMIAOU_N);
}

static PyMethodDef PyBatamiaouGame_methods[] = {
    {"reset", (PyCFunction)PyBatamiaouGame_reset, METH_VARARGS | METH_KEYWORDS, "reset(seed=None)\n\nReset the game (reinitialize the deck). Optionally provide a seed."},
    {"play", (PyCFunction)PyBatamiaouGame_play, METH_NOARGS, "play()\n\nRun a single round of the game"},
    {"run", (PyCFunction)PyBatamiaouGame_run, METH_NOARGS, "run() -> int\n\nRun the whole game and return the number of rounds played"},
    {"deck", (PyCFunction)PyBatamiaouGame_deck, METH_NOARGS, "deck() -> bytes\n\nRead the deck as a bytes buffer"},
    {NULL} /* Sentinel */
};

static PyType_Slot PyBatamiaouGame_slots[] = {
    {Py_tp_new, PyBatamiaouGame_new},
    {Py_tp_init, PyBatamiaouGame_init},
    {Py_tp_methods, PyBatamiaouGame_methods},
    {0, 0}};

static PyType_Spec PyBatamiaouGame_spec = {
    "batamiaou.cbatamiaou.BatamiaouGame",
    sizeof(PyBatamiaouGame),
    0,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    PyBatamiaouGame_slots};

static PyMethodDef module_methods[] = {
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef_Slot batamiaou_slots[] = {
    {0, NULL}};

static struct PyModuleDef batamiaoumodule = {
    PyModuleDef_HEAD_INIT,
    "batamiaou.cbatamiaou",
    "Batamiaou Game Module (C Extension)",
    0,
    module_methods,
    NULL, // m_slots must be NULL if not using multi-phase init
    NULL,
    NULL,
    NULL};

PyMODINIT_FUNC
PyInit_cbatamiaou(void)
{
    PyObject *m;
    PyObject *batamiaou_game_type;

    m = PyModule_Create(&batamiaoumodule);
    if (m == NULL)
        return NULL;

    batamiaou_game_type = PyType_FromSpec(&PyBatamiaouGame_spec);
    if (batamiaou_game_type == NULL)
    {
        Py_DECREF(m);
        return NULL;
    }
    PyModule_AddObject(m, "BatamiaouGame", batamiaou_game_type);
    return m;
}
